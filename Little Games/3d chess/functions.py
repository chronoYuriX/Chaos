##functions.py

#Dependencies
from typing    import Any      as type_any
from typing    import Final    as type_const
from typing    import final    as retoucher_final
import setting

@retoucher_final
class void_set:
    VOID:type_const[str] = 'void'
    @staticmethod
    def voidfunc_totally(*args:type_any):
        pass
    @staticmethod
    def voidfunc_withkey(*args:type_any):
        return args
    @staticmethod
    def voidfunc_retVOID(*args:type_any):
        return void_set.VOID

def check_zone(pos = (0,0),\
              xzone = (0,0),\
              yzone = (0,0)):
    if xzone[0] <= pos[0] <= xzone[1] and \
       yzone[0] <= pos[1] <= yzone[1]:
        return True
    return False

def check_inchessboard(pos = (0,0)     ,\
                       chessboard_width = 9,\
                       chessboard_lenth = 10):
    if 0 <= pos[0] < chessboard_width and \
       0 <= pos[1] < chessboard_lenth:
        return True
    return False

def replacechar(obj,tar,index):
    return obj[:index:] + tar + obj[index+1::]           
