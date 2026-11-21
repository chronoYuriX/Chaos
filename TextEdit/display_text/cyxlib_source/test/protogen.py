#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
genhdr.py —— 零依赖的 C/C++ 头文件"草稿"生成器 (Python 3.7+)

思路：
  1. 预处理：剥注释 / 剥字符串与字符字面量 / 去预处理指令 / 去 raw string
  2. 分词：标识符、数字、符号
  3. 栈式扫描：维护 namespace / class / struct / union / enum / extern"C" 作用域，
     用花括号配对确定作用域边界；模板尖括号单独配对，避免 operator< 之类误判
  4. 在每个作用域内按"声明语句"抽取：
       - 成员函数 / 全局函数原型（含返回值、参数、cv/ref 限定、noexcept、=default 等）
       - 数据成员（类内）
       - typedef / using 别名、类型前向声明
  5. 输出带 include guard 的 .h 草稿

定位：生成"可读的草稿"，减少手写量；模板偏特化、宏生成代码、复杂尾置返回类型
      等极端情况仍需人工核对。生成的头部会带 DRAFT 提示。
"""

import re
import sys

# ----------------------------------------------------------------------------
# 1. 预处理
# ----------------------------------------------------------------------------

def strip_raw_strings(s):
    """去掉 C++11 raw string：R"(...)", R"delim(...)delim" —— 里面可能有任意括号。"""
    out = []
    i = 0
    n = len(s)
    while i < n:
        if s[i] in 'rR' and i + 1 < n and s[i + 1] == '"':
            # 找分隔符
            j = i + 2
            delim = []
            while j < n and s[j] != '(' and (s[j].isalnum() or s[j] in '_-'):
                delim.append(s[j])
                j += 1
            if j < n and s[j] == '(':
                closing = ')' + ''.join(delim) + '"'
                end = s.find(closing, j + 1)
                if end != -1:
                    out.append('""')
                    i = end + len(closing)
                    continue
        out.append(s[i])
        i += 1
    return ''.join(out)


def strip_comments(s):
    """剥注释，但不动字符串内部（字符串已在上一轮被替换成 ""，所以这里安全）。"""
    out = []
    i = 0
    n = len(s)
    while i < n:
        c = s[i]
        if c == '/' and i + 1 < n:
            nxt = s[i + 1]
            if nxt == '/':
                # 行注释
                j = s.find('\n', i)
                if j == -1:
                    break
                out.append(' ')
                i = j
                continue
            if nxt == '*':
                j = s.find('*/', i + 2)
                if j == -1:
                    break
                # 保留换行以维持行号感
                out.append(' ' * 0)
                i = j + 2
                continue
        out.append(c)
        i += 1
    return ''.join(out)


def strip_literals(s):
    """把字符串/字符字面量内容抹掉，保留空壳。支持转义。"""
    out = []
    i = 0
    n = len(s)
    while i < n:
        c = s[i]
        if c == '"':
            i += 1
            while i < n:
                if s[i] == '\\':
                    i += 2
                    continue
                if s[i] == '"':
                    i += 1
                    break
                i += 1
            out.append('""')
            continue
        if c == "'":
            i += 1
            while i < n:
                if s[i] == '\\':
                    i += 2
                    continue
                if s[i] == "'":
                    i += 1
                    break
                i += 1
            out.append("''")
            continue
        # 数字分隔符 / 用户自定义字面量：把 123_xyz 之类简化
        out.append(c)
        i += 1
    return ''.join(out)


def strip_preprocessor(s):
    """去掉预处理指令行；处理反斜杠续行。"""
    # 先合并续行
    s = re.sub(r'\\\n', ' ', s)
    lines = s.split('\n')
    kept = []
    for ln in lines:
        if ln.lstrip().startswith('#'):
            kept.append('')          # 用空行占位，保持后续解析位置不漂
        else:
            kept.append(ln)
    return '\n'.join(kept)


def preprocess(src):
    src = strip_raw_strings(src)
    src = strip_comments(src)
    src = strip_literals(src)
    src = strip_preprocessor(src)
    return src


# ----------------------------------------------------------------------------
# 2. 分词
# ----------------------------------------------------------------------------

TOKEN_RE = re.compile(
    r'''
      (?P<id>    [A-Za-z_]\w* )                     # 标识符 / 关键字
    | (?P<num>   (?: 0[xX][0-9a-fA-F']+
                   | \d[\d']*(?:\.\d*)?
                   | \.\d+ )
                 (?:[eE][+-]?\d+)?[uUlLfF]* )        # 数字
    | (?P<ps>    :: )                                # 作用域解析
    | (?P<arrow> ->\*? | \.\* )                      # ->  ->*  .*
    | (?P<op>    <<=|>>=|->|<=|>=|==|!=|&&|\|\||\+\+|--|<<|>>
                 |[+\-*/%&|^~!=<>] )                 # 多字符运算符
    | (?P<punc>  [{}()\[\];:,.*?] )                  # 单个标点
    | (?P<ws>    \s+ )                               # 空白
    | (?P<other> . )                                 # 兜底
    ''',
    re.VERBOSE,
)


COMPOUND_OPS = {
    ('+', '='): '+=', ('-', '='): '-=', ('*', '='): '*=', ('/', '='): '/=',
    ('%', '='): '%=', ('&', '='): '&=', ('|', '='): '|=', ('^', '='): '^=',
    ('<', '<'): '<<', ('>', '>'): '>>',
    ('<', '='): '<=', ('>', '='): '>=', ('=', '='): '==', ('!', '='): '!=',
    ('&', '&'): '&&', ('|', '|'): '||',
    ('+', '+'): '++', ('-', '-'): '--',
    ('-', '>'): '->',
    ('.', '*'): '.*', ('-', '>', '*'): '->*',
}


def merge_operators(toks):
    """把被 tokenizer 拆开的复合运算符重新合并（+=  <<=  ->*  等）。"""
    out = []
    i = 0
    n = len(toks)
    while i < n:
        kind, val = toks[i]
        merged = None
        if kind == 'op' or kind == 'punc':
            nxt = toks[i + 1][1] if i + 1 < n else ''
            key = (val, nxt)
            if key in COMPOUND_OPS:
                # <<= 之类三字符
                if key in (('+', '='), ('-', '='), ('*', '='), ('/', '='), ('%', '='),
                           ('&', '='), ('|', '='), ('^', '='), ('<', '='), ('>', '='),
                           ('=', '='), ('!', '='), ('&', '&'), ('|', '|'),
                           ('+', '+'), ('-', '-'), ('-', '>')):
                    merged = COMPOUND_OPS[key]
                    i += 2
                elif key in (('<', '<'), ('>', '>')):
                    # 可能是 << 或 <<=
                    nn = toks[i + 2][1] if i + 2 < n else ''
                    if nn == '=':
                        merged = COMPOUND_OPS[key] + '='
                        i += 3
                    else:
                        merged = COMPOUND_OPS[key]
                        i += 2
                elif key == ('.', '*'):
                    merged = '.*'
                    i += 2
        if merged is None:
            out.append((kind, val))
            i += 1
        else:
            out.append(('op', merged))
    return out


def tokenize(src):
    toks = []
    for m in TOKEN_RE.finditer(src):
        kind = m.lastgroup
        val = m.group()
        if kind == 'ws':
            continue
        toks.append((kind, val))
    return merge_operators(toks)


# ----------------------------------------------------------------------------
# 3. 关键字集合
# ----------------------------------------------------------------------------

SCOPE_KW = {'namespace', 'class', 'struct', 'union', 'enum'}
CTRL_KW = {'if', 'for', 'while', 'switch', 'catch', 'do'}
# 出现在声明开头、但不该被当成函数名的关键字
DECL_NOISE = {
    'public', 'private', 'protected', 'virtual', 'static', 'inline', 'explicit',
    'friend', 'typedef', 'using', 'constexpr', 'consteval', 'constinit',
    'volatile', 'mutable', 'extern', 'register', 'thread_local', 'const',
    'typename', 'template', 'requires', 'noexcept', 'throw', 'final', 'override',
    'default', 'delete', 'alignas', 'and', 'or', 'not', 'decltype',
}


# ----------------------------------------------------------------------------
# 4. 声明语句解析
# ----------------------------------------------------------------------------

def find_matching(toks, i, open_ch, close_ch):
    """从 toks[i] == open_ch 开始，返回匹配的 close_ch 下标；找不到返回 -1。
       会跳过嵌套的同类括号。"""
    assert toks[i][1] == open_ch
    depth = 0
    n = len(toks)
    while i < n:
        v = toks[i][1]
        if v == open_ch:
            depth += 1
        elif v == close_ch:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def split_template_name(parts):
    """把 ['vector','<','int','>'] 这类合成 'vector<int>'；'::' 也连起来。"""
    s = ''
    for p in parts:
        if p == '::':
            s += '::'
        elif p in ('<', '>', ',', '&', '*', '&&'):
            s += p
        else:
            if s and s[-1] not in '<:, &*' and p not in '<>,&*':
                s += ' '
            s += p
    return s.strip()


def extract_decls(stmt_toks):
    """把一条"声明语句"（不含分号、不含花括号体）拆成一个或多个声明项。
       返回 [(type_tokens, name, suffix_tokens)]，suffix 含 = 初始值等。"""
    # 按顶层逗号切分（跳过 <> () [] 内的逗号）
    groups = []
    cur = []
    depth = 0
    for t in stmt_toks:
        v = t[1]
        if v in '<(':
            depth += 1
        elif v in ')>':
            depth -= 1
        if v == ',' and depth == 0:
            groups.append(cur)
            cur = []
            continue
        cur.append(t)
    if cur:
        groups.append(cur)

    results = []
    for g in groups:
        if not g:
            continue
        # 找到"名字"：第一个标识符，其后不是 :: 模板参数开头
        # 简化：找最后一个"裸标识符"，它前面是类型、后面是 ( / [ / = / ; 结尾
        name_idx = None
        for k in range(len(g) - 1, -1, -1):
            kind, val = g[k]
            if kind != 'id':
                continue
            if val in DECL_NOISE:
                continue
            # 后面必须是 ( [ = 或结束
            nxt = g[k + 1][1] if k + 1 < len(g) else ';'
            if nxt in ('(', '[', '=', ';', ':'):
                name_idx = k
                break
        if name_idx is None:
            results.append((g, '', []))
            continue
        type_toks = g[:name_idx]
        name = g[name_idx][1]
        suffix = g[name_idx + 1:]
        results.append((type_toks, name, suffix))
    return results


PUNCT_NO_SPACE_BEFORE = {'<', '(', '['}
PUNCT_NO_SPACE_AFTER = {'(', '['}


def render_type(toks):
    """把 token 序列还原成源码文本，标点粘连规则贴近 C++ 习惯。"""
    out = ''
    prev = ''
    for kind, val in toks:
        if val in (',', ';'):
            out = out.rstrip() + val + ' '
            prev = val
            continue
        if val == '::':
            out = out.rstrip() + '::'
            prev = '::'
            continue
        if val in ('*', '&', '&&'):
            # 绑定左侧类型：T& / T* / T&&
            out = out.rstrip() + val + ' '
            prev = val
            continue
        if val in PUNCT_NO_SPACE_BEFORE:   # '(' '[' '<'
            out = out.rstrip() + val
            prev = val
            continue
        if prev == '(' and kind == 'id':
            # 函数指针名紧贴左括号：(*Metric)
            out += val
            prev = val
            continue
        if prev in PUNCT_NO_SPACE_AFTER or prev in ('::', '*', '&', '&&'):
            out += val
            prev = val
            continue
        if out and not out.endswith(' '):
            out += ' '
        out += val
        prev = val
    out = out.replace(' :: ', '::')
    return out.strip()


def is_function_decl(suffix):
    """suffix 以 '(' 开头 → 函数。还要排除 if/for/while/switch/catch 语句。"""
    if not suffix:
        return False
    return suffix[0][1] == '('


def collect_params(paren_toks):
    """把括号内 token 列表还原成参数列表字符串；去掉变量名只留类型（可选）。"""
    # 去掉最外层括号
    inner = paren_toks[1:-1]
    if not inner:
        return ''
    # 按顶层逗号切
    parts = []
    cur = []
    depth = 0
    for t in inner:
        v = t[1]
        if v in '<(':
            depth += 1
        elif v in ')>':
            depth -= 1
        if v == ',' and depth == 0:
            parts.append(cur)
            cur = []
            continue
        cur.append(t)
    if cur:
        parts.append(cur)
    out = []
    for p in parts:
        txt = render_type(p)
        # 去掉右端粘连标点前的空格：'other )' -> 'other)'
        txt = re.sub(r'\s+([)\]>])', r'\1', txt)
        # 尝试去掉最后一个标识符（参数名）
        toks = p
        if toks and toks[-1][0] == 'id' and toks[-1][1] not in DECL_NOISE:
            # 保留类型，把参数名换成空 → 但为了可读，这里选择保留名字
            pass
        out.append(txt)
    text = ', '.join(out)
    return re.sub(r'\s+([)\]>])', r'\1', text)


def strip_leading_noise(type_toks):
    """去掉类型前修饰里不适合出现在头文件草稿中的：static / register / thread_local。"""
    drop = {'static', 'register', 'thread_local', 'auto'}
    res = [t for t in type_toks if not (t[0] == 'id' and t[1] in drop)]
    return res


# ----------------------------------------------------------------------------
# 5. 主扫描器
# ----------------------------------------------------------------------------

class Scope:
    def __init__(self, kind, name, parent):
        self.kind = kind        # namespace/class/struct/union/enum/extern_c/function/block
        self.name = name        # 可能含模板参数串
        self.parent = parent
        self.children = []
        self.funcs = []         # 本作用域内的函数原型字符串
        self.members = []       # 数据成员/枚举值/别名等
        if parent is not None:
            parent.children.append(self)

    def qualname(self):
        parts = []
        cur = self
        while cur is not None and cur.kind != 'block' and cur.kind != 'function':
            if cur.name:
                parts.append(cur.name)
            cur = cur.parent
        return '::'.join(reversed(parts))

    def is_type_scope(self):
        return self.kind in ('class', 'struct', 'union')


class Scanner:
    def __init__(self, toks):
        self.toks = toks
        self.i = 0
        self.n = len(toks)
        self.root = Scope('namespace', '', None)   # 匿名全局命名空间
        self.stack = [self.root]
        self.pending_scope = None   # (kind, name_tokens)
        self.stmt = []              # 当前语句累积 token
        self.at_line_start = True

    # -- helpers --
    def peek(self, k=0):
        j = self.i + k
        return self.toks[j] if j < self.n else (None, '')

    def next(self):
        t = self.peek()
        self.i += 1
        return t

    def top(self):
        return self.stack[-1]

    # -- 主循环 --
    def run(self):
        while self.i < self.n:
            kind, val = self.next()

            if val == '{':
                self.brace_open()
                continue
            if val == '}':
                self.brace_close()
                continue
            if val == ';':
                self.flush_stmt()
                continue
            if val == ':':
                # 访问说明符 / 位域 / 三目；访问说明符直接跳过
                self.stmt.append((kind, val))
                # 若当前语句只有 public/private/protected → 清掉
                if len(self.stmt) >= 1 and self.stmt[-1][1] == ':' and \
                   len(self.stmt) >= 2 and self.stmt[-2][1] in ('public', 'private', 'protected'):
                    self.stmt = []
                continue
            if kind == 'id' and val in SCOPE_KW:
                # 可能开始一个作用域声明
                self.start_scope_decl(val)
                continue
            if kind == 'id' and val == 'extern':
                # extern "C" { ... }
                self.stmt.append((kind, val))
                nk, nv = self.peek()
                if nv == '"':
                    # 吃掉 "C"
                    self.next()  # 引号
                    if self.peek()[1] == 'C':
                        self.next()
                    if self.peek()[1] == '"':
                        self.next()
                    self.pending_scope = ('extern_c', [])
                    self.stmt = []
                continue
            if kind == 'id' and val == 'template':
                # 跳过 template<...> 里的内容，不进 stmt（避免污染类型）
                self.skip_template_head()
                continue

            self.stmt.append((kind, val))

        # 文件结束
        self.flush_stmt()

    # -- 作用域 --
    def start_scope_decl(self, kw):
        # 收集名字（可能含 ::、模板参数）
        name_parts = []
        while self.i < self.n:
            k, v = self.peek()
            if v in ('{', ';', ':'):
                break
            if v == '(':
                # 是函数声明，不是作用域
                self.stmt.append((kind_of(kw), kw))
                self.stmt.extend(name_parts)
                return
            name_parts.append((k, v))
            self.next()
        # 名字末尾可能带基类列表起始 ':' 已被排除
        name = split_template_name([v for _, v in name_parts])
        if kw == 'enum':
            # 枚举体不建子作用域：枚举值作为当前作用域的 member
            self.pending_scope = None
            self.stmt.append((kind_of(kw), kw))
            self.stmt.extend(name_parts)
            return
        self.pending_scope = (kw, name)
        # 若这同时是一条完整声明（枚举前置声明 enum E;），stmt 保留以便后续处理
        if self.peek()[1] == ';':
            self.stmt.append((kind_of(kw), kw))
            self.stmt.extend(name_parts)

    def skip_template_head(self):
        # 当前已吃掉 'template'；期待 '<'，跳到匹配 '>'
        if self.peek()[1] != '<':
            return
        self.next()  # '<'
        depth = 1
        while self.i < self.n and depth > 0:
            k, v = self.next()
            if v == '<':
                depth += 1
            elif v == '>':
                depth -= 1
            elif v == '>>':
                depth -= 2
        # 不把 template 头加进 stmt

    def brace_open(self):
        # 决定这个 '{' 开启什么
        prev = self.stmt[-1][1] if self.stmt else ''
        pend = self.pending_scope
        if pend and pend[0] == 'enum':
            # 不应发生（enum 已在 start_scope_decl 里清空），兜底
            self.pending_scope = None
        if pend:
            kind, name = pend
            sc = Scope(kind, name, self.top())
            self.stack.append(sc)
            self.pending_scope = None
            self.stmt = []
            return
        if prev == ')':
            # 函数体
            sc = Scope('function', self.last_func_name(), self.top())
            self.stack.append(sc)
            self.stmt = []
            return
        if prev == '=' or prev == ',' or prev == '(':
            # 初始化列表 / 聚合初始化：不压作用域，但也不能留着 stmt
            self.stmt = []
            return
        if prev == '' :
            # 裸块
            self.stack.append(Scope('block', '', self.top()))
            self.stmt = []
            return
        # 其他情况（如 namespace 已由 pending 处理；lambda 等）当普通块
        self.stack.append(Scope('block', '', self.top()))
        self.stmt = []

    def brace_close(self):
        if len(self.stack) > 1:
            self.stack.pop()
        self.stmt = []
        self.pending_scope = None

    def last_func_name(self):
        # 从当前 stmt 里找函数名
        for k in range(len(self.stmt) - 1, -1, -1):
            if self.stmt[k][1] == ')':
                # 找 '(' 前的标识符
                j = k - 1
                depth = 0
                while j >= 0:
                    v = self.stmt[j][1]
                    if v == ')':
                        depth += 1
                    elif v == '(':
                        if depth == 0:
                            break
                        depth -= 1
                    j -= 1
                if j - 1 >= 0 and self.stmt[j - 1][0] == 'id':
                    return self.stmt[j - 1][1]
                return ''
        return ''

    # -- 语句处理 --
    def flush_stmt(self):
        toks = self.stmt
        self.stmt = []
        if not toks:
            return
        # 去掉开头访问控制（防御）
        while toks and toks[0][1] in ('public', 'private', 'protected') and \
                (len(toks) == 1 or toks[1][1] != ':' or True):
            # 访问说明单独成句时 toks == ['public'] 之类
            if len(toks) == 1:
                return
            toks = toks[1:]
        if not toks:
            return

        top = self.top()

        # using / typedef
        if toks[0][1] == 'using' and not any(t[1] == '=' for t in toks):
            # using namespace std; 之类，跳过
            return

        # 寻找"函数名 + (" 结构
        func_pos = None
        for k in range(len(toks) - 1, -1, -1):
            if toks[k][1] == '(' and k > 0 and toks[k - 1][0] == 'id':
                nm = toks[k - 1][1]
                if nm in CTRL_KW or nm in ('sizeof', 'alignof', 'typeid',
                                           'static_cast', 'dynamic_cast',
                                           'const_cast', 'reinterpret_cast'):
                    continue
                # 排除 operator() 情形：前一 token 是 operator
                if k - 2 >= 0 and toks[k - 2][1] == 'operator':
                    pass
                func_pos = k
                break

        if func_pos is not None and not self.is_typedef_or_using(toks):
            self.emit_function(toks, func_pos)
            return

        # 否则作为"成员/类型声明"
        self.emit_member(toks)

    def is_typedef_or_using(self, toks):
        return toks and toks[0][1] in ('typedef', 'using')

    def emit_function(self, toks, func_pos):
        name = toks[func_pos - 1][1]
        # 处理 operator 重载名：operator 后面的运算符可能由多个 token 组成（+=  ->  []  () 等）
        if func_pos - 2 >= 0 and toks[func_pos - 2][1] == 'operator':
            j = func_pos - 1
            parts = []
            while j < len(toks) and toks[j][1] != '(':
                parts.append(toks[j][1])
                j += 1
            name = 'operator' + ''.join(parts)
            # 重新定位参数括号起点
            func_pos = j
        # 找参数括号范围
        close = find_matching(toks, func_pos, '(', ')')
        if close == -1:
            return
        params = collect_params(toks[func_pos:close + 1])
        suffix = toks[close + 1:]
        # 返回值 + 名字前部分
        head = render_type(strip_leading_noise(toks[:func_pos - 1]))
        # 拼 cv / ref / noexcept / override / =0 / =default 等
        tail = render_type(suffix)
        # 去掉初始化列表 ':' 之后的（构造函数）
        if tail.startswith(':'):
            tail = ''
        params = re.sub(r'\s+([)\]>])', r'\1', params).strip()
        proto = f'{head} {name}({params})'.strip()
        if tail and not tail.startswith('{'):
            proto += ' ' + tail
        proto += ';'
        self.top().funcs.append(proto)

    def emit_member(self, toks):
        top = self.top()
        # typedef / using 别名
        if toks[0][1] in ('typedef', 'using'):
            txt = render_type(toks)
            # 紧凑化函数指针形态：'(* Name )' -> '(*Name)'
            txt = re.sub(r'\(\s*\*\s*(\w+)\s*\)', r'(*\1)', txt)
            txt = re.sub(r'\s+([)\]>])', r'\1', txt)
            top.members.append(txt + ';')
            return
        # 枚举值已在 enum 作用域里当 member 处理（此处不会进）
        # 数据成员 / 前向声明：若有 '=' ，截断掉初始化表达式（宏调用可能含括号/逗号）
        eq_idx = None
        depth = 0
        for k, t in enumerate(toks):
            if t[1] in '<(':
                depth += 1
            elif t[1] in ')>':
                depth -= 1
            elif t[1] == '=' and depth == 0:
                eq_idx = k
                break
        if eq_idx is not None:
            toks = toks[:eq_idx]
        txt = render_type(strip_leading_noise(toks))
        if not txt:
            return
        # 排除孤立关键字
        if txt in ('public', 'private', 'protected', 'extern'):
            return
        top.members.append(txt + ';')


def kind_of(kw):
    return kw


# ----------------------------------------------------------------------------
# 6. 输出
# ----------------------------------------------------------------------------

def render_scope(sc, out, depth=0):
    ind = '    ' * depth
    if sc.kind in ('class', 'struct', 'union'):
        kw = sc.kind
        header = f'{ind}{kw} {sc.name} {{'
        out.append(header)
        body = []
        if sc.members:
            body.append(f'{ind}    // ---- members ----')
            for m in sc.members:
                body.append(f'{ind}    {m}')
        if sc.funcs:
            if body:
                body.append('')
            body.append(f'{ind}    // ---- methods ----')
            for f in sc.funcs:
                body.append(f'{ind}    {f}')
        out.extend(body)
        # 嵌套类型
        for c in sc.children:
            render_scope(c, out, depth + 1)
        out.append(f'{ind}}};')
    elif sc.kind == 'namespace':
        if sc.name:
            out.append(f'{ind}namespace {sc.name} {{')
        else:
            out.append(f'{ind}/* global namespace */ {{')
        for m in sc.members:
            out.append(f'{ind}    {m}')
        for f in sc.funcs:
            out.append(f'{ind}    {f}')
        for c in sc.children:
            render_scope(c, out, depth + 1)
        if sc.name:
            out.append(f'{ind}}} // namespace {sc.name}')
        else:
            out.append(f'{ind}}}')
    elif sc.kind == 'enum':
        header = f'{ind}enum {sc.name} {{'.strip()
        out.append(header)
        for m in sc.members:
            out.append(f'{ind}    {m}')
        out.append(f'{ind}}};')
    elif sc.kind == 'extern_c':
        out.append(f'{ind}extern "C" {{')
        for f in sc.funcs:
            out.append(f'{ind}    {f}')
        for c in sc.children:
            render_scope(c, out, depth + 1)
        out.append(f'{ind}}}')
    else:
        # function / block：不输出
        for c in sc.children:
            render_scope(c, out, depth)


def generate(files):
    guard = 'GENERATED_H_DRAFT'
    out = []
    out.append(f'#ifndef {guard}')
    out.append(f'#define {guard}')
    out.append('')
    out.append('// === 由 genhdr.py 自动生成的草稿头文件 ===')
    out.append('// 请人工核对：模板特化、宏生成代码、运算符重载、尾置返回类型、')
    out.append('// 复杂继承与 using 声明等场景可能不完整或需要调整。')
    out.append('')
    for path in files:
        try:
            src = open(path, encoding='utf-8', errors='replace').read()
        except OSError as e:
            out.append(f'// !!! 无法读取 {path}: {e}')
            continue
        out.append(f'// ===================== {path} =====================')
        toks = tokenize(preprocess(src))
        sc = Scanner(toks)
        sc.run()
        body = []
        # 全局命名空间：先 members 再 funcs，再 children
        for m in sc.root.members:
            body.append(f'    {m}')
        for f in sc.root.funcs:
            body.append(f'    {f}')
        for c in sc.root.children:
            render_scope(c, body, 1)
        if not body:
            body.append('    // (未提取到声明)')
        out.extend(body)
        out.append('')
    out.append(f'#endif /* {guard} */')
    out.append('')
    return '\n'.join(out)


# ----------------------------------------------------------------------------
# 7. 入口
# ----------------------------------------------------------------------------

def main(argv):
    if len(argv) < 2:
        sys.stderr.write(
            '用法: python genhdr.py <源文件...> [-o 输出.h]\n'
            '示例: python genhdr.py foo.cpp bar.cpp -o draft.h\n'
        )
        return 2
    files = []
    out_path = None
    i = 1
    while i < len(argv):
        if argv[i] == '-o' and i + 1 < len(argv):
            out_path = argv[i + 1]
            i += 2
        else:
            files.append(argv[i])
            i += 1
    text = generate(files)
    if out_path:
        with open(out_path, 'w', encoding='ANSI') as f:
            f.write(text)
        sys.stderr.write(f'已写入 {out_path}\n')
    else:
        sys.stdout.write(text)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
