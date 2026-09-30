class rope_node:
    def __init__(self, string):
        self.string = string
        self.is_string = True
        self.length = len(string)
    def split(self, index):
        if index == "LOL":
            return rope_node(self.string), None
        return rope_node(self.string[:index]), rope_node(self.string[index:])
    def transform_into_node(self, left, right):
        self.left = left
        self.right = right
        self.is_string = False
        self.left_length = left.length

class rope:
    def __init__(self, string):
        self.root = rope_node(string)
    def insert_iter(self, current_root, relative_index, string):
        if current_root.is_string:
            left, right = current_root.split(relative_index)
            current_root.transform_into_node(left, right)
            left_left, left_right = current_root.left.split("LOL")
            current_root.left.transform_into_node(left_left, left_right)
            current_root.left.right = rope_node(string)
            current_root.left_length = current_root.left.left_length + len(string)
            del current_root.string
        else:
            if relative_index < current_root.left_length:
                self.insert_iter(current_root.left, relative_index, string)
                current_root.left_length += len(string)
            else:
                self.insert_iter(current_root.right, relative_index - current_root.left_length, string)
    def insert(self, index, string):
        self.insert_iter(self.root, index, string)
    def get_iter(self, current_root, dest, tabs = 0):
        print("  " * tabs, end = '')
        if current_root.is_string:
            dest.append(current_root.string)
            print(current_root.string)
        else:
            print("<node, left_length = %d>" % current_root.left_length)
            self.get_iter(current_root.left, dest, tabs + 1)
            self.get_iter(current_root.right, dest, tabs + 1)
    def get(self):
        output = []
        self.get_iter(self.root, output)
        return output

r = rope("12345678")
print(''.join(r.get()))
r.insert(3, "[insert]")
print(''.join(r.get()))
r.insert(4, "(double insert)")
print(''.join(r.get()))

        
