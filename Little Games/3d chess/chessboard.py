##chessboard.py

import chars
import functions
import CHCMR7
import setting

class pieces:
    def __init__(self                                           ,\
                 name = functions.void_set.VOID                 ,\
                 value = 0                                      ,\
                 placement = functions.void_set.voidfunc_totally,\
                 pos = (0,0)                                    ,\
                 side = 0):
        self.name = name
        self.value = value
        self.placement = placement
        self.pos = pos
        self.side = side

    def check(self):
        return self.placement(self.pos[0],self.pos[1],self.side)

class chessboard:
    def __init__(self                         ,\
                 chessboard_width = 9         ,\
                 chessboard_lenth = 10        ,\
                 chessboard_river = 4         ,\
                 chessboard_void = 0          ,\
                 check_stumbling_horse = True ,\
                 check_stumbling_vizier = True,\
                 players = (1,2)              ,\
                 admirals = chars.admirals    ,\
                 player_name = (chars.XiangYu,chars.LiuBang)):
        self.chessboard_width = chessboard_width
        self.chessboard_lenth = chessboard_lenth
        self.chessboard_river = chessboard_river
        self.chessboard_void  = chessboard_void
        self.check_stumbling_horse  = check_stumbling_horse
        self.check_stumbling_vizier = check_stumbling_vizier
        self.players = players
        self.admirals = admirals
        self.player_name = player_name
        renderer_settings = setting.readsettings('3d')
        self.window_x = renderer_settings['window_x']
        self.window_y = renderer_settings['window_y']
        self.renderer = CHCMR7.renderer_nIMG((renderer_settings['window_x'],\
                                              renderer_settings['window_y'],),\
                                             renderer_settings['magnify'],\
                                             renderer_settings['bgcolor'],\
                                             renderer_settings['range'  ],\
                                             renderer_settings['light'  ],\
                                             renderer_settings['bglight'])
        self.renderer.heading = (renderer_settings['cam_angle'],\
                                 renderer_settings['cam_pitch'])
        self.chessboard_floor = CHCMR7.triangles()
        self.chessboard_floor.clear()
        self.chessboard_floor.addsurface((renderer_settings['board_floor_0'],\
                                          renderer_settings['board_floor_1'],\
                                          renderer_settings['board_floor_2'],\
                                          renderer_settings['board_floor_3']),\
                                         renderer_settings['board_floor_color'])
        self.chessboard_top   = (((renderer_settings['board_top_0'  ],\
                                   renderer_settings['board_top_1'  ],\
                                   renderer_settings['board_top_2'  ],\
                                   renderer_settings['board_top_3'  ]),\
                                  renderer_settings['board_top_color'  ]))
        self.chessboard_front = (((renderer_settings['board_front_0'],\
                                   renderer_settings['board_front_1'],\
                                   renderer_settings['board_front_2'],\
                                   renderer_settings['board_front_3']),\
                                  renderer_settings['board_front_color']))
        self.chessboard_right = (((renderer_settings['board_right_0'],\
                                   renderer_settings['board_right_1'],\
                                   renderer_settings['board_right_2'],\
                                   renderer_settings['board_right_3']),\
                                  renderer_settings['board_right_color']))
        self.chessboard_left  = (((renderer_settings['board_left_0'],\
                                   renderer_settings['board_left_1'],\
                                   renderer_settings['board_left_2'],\
                                   renderer_settings['board_left_3']),\
                                  renderer_settings['board_left_color']))
        self.chessboard_back  = (((renderer_settings['board_back_0'],\
                                   renderer_settings['board_back_1'],\
                                   renderer_settings['board_back_2'],\
                                   renderer_settings['board_back_3']),\
                                  renderer_settings['board_back_color']))
        self.chessboard_base = CHCMR7.triangles()
        self.chessboard_base.clear()
        self.chessboard_base.addsurface(self.chessboard_top  [0],\
                               self.chessboard_top  [1])
        self.chessboard_base.addsurface(self.chessboard_front[0],\
                               self.chessboard_front[1])
        self.chessboard_base.addsurface(self.chessboard_right[0],\
                               self.chessboard_right[1])
        self.chessboard_base.addsurface(self.chessboard_left [0],\
                               self.chessboard_left [1])
        self.chessboard_base.addsurface(self.chessboard_back [0],\
                               self.chessboard_back [1])
        self.chessboard_texture_VL0 = (renderer_settings['VL00'],\
                                       renderer_settings['VL01'],\
                                       renderer_settings['VL02'],\
                                       renderer_settings['VL03'])
        self.chessboard_texture_VL1 = (renderer_settings['VL10'],\
                                       renderer_settings['VL11'],\
                                       renderer_settings['VL12'],\
                                       renderer_settings['VL13'])
        self.chessboard_texture_VL2 = (renderer_settings['VL20'],\
                                       renderer_settings['VL21'],\
                                       renderer_settings['VL22'],\
                                       renderer_settings['VL23'])
        self.chessboard_texture_VM0 = (renderer_settings['VM00'],\
                                       renderer_settings['VM01'],\
                                       renderer_settings['VM02'],\
                                       renderer_settings['VM03'])
        self.chessboard_texture_VM1 = (renderer_settings['VM10'],\
                                       renderer_settings['VM11'],\
                                       renderer_settings['VM12'],\
                                       renderer_settings['VM13'])
        self.chessboard_texture_VM2 = (renderer_settings['VM20'],\
                                       renderer_settings['VM21'],\
                                       renderer_settings['VM22'],\
                                       renderer_settings['VM23'])
        self.chessboard_texture_VR0 = (renderer_settings['VR00'],\
                                       renderer_settings['VR01'],\
                                       renderer_settings['VR02'],\
                                       renderer_settings['VR03'])
        self.chessboard_texture_VR1 = (renderer_settings['VR10'],\
                                       renderer_settings['VR11'],\
                                       renderer_settings['VR12'],\
                                       renderer_settings['VR13'])
        self.chessboard_texture_VR2 = (renderer_settings['VR20'],\
                                       renderer_settings['VR21'],\
                                       renderer_settings['VR22'],\
                                       renderer_settings['VR23'])
        self.chessboard_texture_HF0 = (renderer_settings['HF00'],\
                                       renderer_settings['HF01'],\
                                       renderer_settings['HF02'],\
                                       renderer_settings['HF03'])
        self.chessboard_texture_HF1 = (renderer_settings['HF10'],\
                                       renderer_settings['HF11'],\
                                       renderer_settings['HF12'],\
                                       renderer_settings['HF13'])
        self.chessboard_texture_HF2 = (renderer_settings['HF20'],\
                                       renderer_settings['HF21'],\
                                       renderer_settings['HF22'],\
                                       renderer_settings['HF23'])
        self.chessboard_texture_HC0 = (renderer_settings['HC00'],\
                                       renderer_settings['HC01'],\
                                       renderer_settings['HC02'],\
                                       renderer_settings['HC03'])
        self.chessboard_texture_HC1 = (renderer_settings['HC10'],\
                                       renderer_settings['HC11'],\
                                       renderer_settings['HC12'],\
                                       renderer_settings['HC13'])
        self.chessboard_texture_HC2 = (renderer_settings['HC20'],\
                                       renderer_settings['HC21'],\
                                       renderer_settings['HC22'],\
                                       renderer_settings['HC23'])
        self.chessboard_texture_HC3 = (renderer_settings['HC30'],\
                                       renderer_settings['HC31'],\
                                       renderer_settings['HC32'],\
                                       renderer_settings['HC33'])
        self.chessboard_texture_HB0 = (renderer_settings['HB00'],\
                                       renderer_settings['HB01'],\
                                       renderer_settings['HB02'],\
                                       renderer_settings['HB03'])
        self.chessboard_texture_HB1 = (renderer_settings['HB10'],\
                                       renderer_settings['HB11'],\
                                       renderer_settings['HB12'],\
                                       renderer_settings['HB13'])
        self.chessboard_texture_HB2 = (renderer_settings['HB20'],\
                                       renderer_settings['HB21'],\
                                       renderer_settings['HB22'],\
                                       renderer_settings['HB23'])
        self.chessboard_texture_color = renderer_settings['board_texture_color']
        self.chessboard_texture = CHCMR7.triangles()
        self.chessboard_texture.clear()
        self.chessboard_texture.addsurface(self.chessboard_texture_VL0,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VL1,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VL2,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VM0,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VM1,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VM2,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VR0,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VR1,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_VR2,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HF0,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HF1,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HF2,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HC0,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HC1,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HC2,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HC3,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HB0,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HB1,\
                                           self.chessboard_texture_color)
        self.chessboard_texture.addsurface(self.chessboard_texture_HB2,\
                                           self.chessboard_texture_color)
        self.chessboard_mask_A = (renderer_settings['MASK0'],\
                                  renderer_settings['MASK1'],\
                                  renderer_settings['MASK2'],\
                                  renderer_settings['MASK3'])
        self.chessboard_mask = CHCMR7.triangles()
        self.chessboard_mask.clear()
        self.chessboard_mask.addsurface(self.chessboard_mask_A,\
                                        renderer_settings['board_top_color'])
        self.render_all()
        self.pieces_top   = (renderer_settings['pieces_top_0'  ],\
                             renderer_settings['pieces_top_1'  ],\
                             renderer_settings['pieces_top_2'  ],\
                             renderer_settings['pieces_top_3'  ])
        self.pieces_top_color   = renderer_settings['pieces_top_color'  ]
        self.pieces_front = (renderer_settings['pieces_front_0'],\
                             renderer_settings['pieces_front_1'],\
                             renderer_settings['pieces_front_2'],\
                             renderer_settings['pieces_front_3'])
        self.pieces_front_color = renderer_settings['pieces_front_color']
        self.pieces_right = (renderer_settings['pieces_right_0'],\
                             renderer_settings['pieces_right_1'],\
                             renderer_settings['pieces_right_2'],\
                             renderer_settings['pieces_right_3'])
        self.pieces_right_color = renderer_settings['pieces_right_color']
        self.pieces_left  = (renderer_settings['pieces_left_0' ],\
                             renderer_settings['pieces_left_1' ],\
                             renderer_settings['pieces_left_2' ],\
                             renderer_settings['pieces_left_3' ])
        self.pieces_left_color  = renderer_settings['pieces_left_color' ]
        self.pieces_back  = (renderer_settings['pieces_back_0' ],\
                             renderer_settings['pieces_back_1' ],\
                             renderer_settings['pieces_back_2' ],\
                             renderer_settings['pieces_back_3' ])
        self.pieces_back_color  = renderer_settings['pieces_back_color' ]
        self.pieces_magnify_x = renderer_settings['pieces_magnify_x']
        self.pieces_magnify_z = renderer_settings['pieces_magnify_z']
        self.pieces_add_x = renderer_settings['pieces_add_x']
        self.pieces_add_z = renderer_settings['pieces_add_z']
        self.render_pieces = CHCMR7.triangles()
        self.charsettings = {chars.admirals[0]    :renderer_settings['jiang' ],\
                             chars.admirals[1]    :renderer_settings['shuai' ],\
                             chars.pawn_alpha     :renderer_settings['bing'  ],\
                             chars.pawn_beta      :renderer_settings['zu'    ],\
                             chars.cannon_alpha   :renderer_settings['paoA'  ],\
                             chars.cannon_beta    :renderer_settings['paoB'  ],\
                             chars.chariot_alpha  :renderer_settings['che'   ],\
                             chars.chariot_beta   :renderer_settings['jv'    ],\
                             chars.steed_alpha    :renderer_settings['maA'   ],\
                             chars.steed_beta     :renderer_settings['maB'   ],\
                             chars.vizier_alpha   :renderer_settings['xiangA'],\
                             chars.vizier_beta    :renderer_settings['xiangB'],\
                             chars.chevalier_alpha:renderer_settings['shiA'  ],\
                             chars.chevalier_beta :renderer_settings['shiB'  ]}
        self.charcodes = {'*':(renderer_settings['full_A'],\
                               renderer_settings['full_B']),\
                          '+':(renderer_settings['half_A'],\
                               renderer_settings['half_B']),\
                          '-':(renderer_settings['void_A'],\
                               renderer_settings['void_B'])}
        self.char_col = renderer_settings['col']
        self.char_lin = renderer_settings['lin']
        self.char_shift_x = renderer_settings['shift_x']
        self.char_shift_z = renderer_settings['shift_z']
        self.char_total_y = renderer_settings['total_y']
        self.char_size = renderer_settings['pixel_size']
        self.render_fonts = CHCMR7.triangles()
        self.render_fonts.clear()
        self.center = ((renderer_settings['pieces_top_0'][0] + \
                        renderer_settings['pieces_top_1'][0] + \
                        renderer_settings['pieces_top_2'][0] + \
                        renderer_settings['pieces_top_3'][0])/4,\
                       (renderer_settings['pieces_top_0'][1] + \
                        renderer_settings['pieces_top_1'][1] + \
                        renderer_settings['pieces_top_2'][1] + \
                        renderer_settings['pieces_top_3'][1])/4,\
                       (renderer_settings['pieces_top_0'][2] + \
                        renderer_settings['pieces_top_1'][2] + \
                        renderer_settings['pieces_top_2'][2] + \
                        renderer_settings['pieces_top_3'][2])/4)
        self.onkey = {}
        gamesettings = setting.readsettings('config')
        self.onkey_range = gamesettings['range'] ** 2
        self.renderer.GUI.canvas.bind('<Button-1>',self.click)
        self.select = (-1,-1)
        self.player_index = 0
        self.gameloop = True
        print('%s%s'%(self.player_name[self.player_index % len(self.players)],\
                      chars.round_belongs))

    def click(self,event):
        if not self.gameloop:
            return
        if self.window_y - 21 < event.y < self.window_y:
            if 0 < event.x < 21:
                self.renderer.heading = (self.renderer.heading[0] - 0.05,\
                                         self.renderer.heading[1])
            elif 22 < event.x < 43:
                self.renderer.heading = (self.renderer.heading[0] + 0.05,\
                                         self.renderer.heading[1])
            elif self.window_x - 21 < event.x < self.window_x:
                self.renderer.heading = (self.renderer.heading[0],\
                                         self.renderer.heading[1] - 0.05)
            elif self.window_x - 43 < event.x < self.window_x - 22:
                self.renderer.heading = (self.renderer.heading[0],\
                                         self.renderer.heading[1] + 0.05)
            ##self.render_all()
            self.draw(False)
        elif 0 < event.y < 21:
            if 0 < event.x < 21:
                with open('memory.txt','w',encoding = 'utf-16') as f:
                    f.write(self.draw(True))
                    print(chars.saved)
        else:
            foundclick = False
            for key,value in self.onkey.items():
                self.renderer.GUI.line((key[0] - 3,key[1] - 3),\
                                       (key[0] + 3,key[1] + 3),\
                                       (0,255,0))
                self.renderer.GUI.line((key[0] + 3,key[1] - 3),\
                                       (key[0] - 3,key[1] + 3),\
                                       (0,255,0))
                if foundclick:
                    continue
                if (event.x - key[0]) ** 2 + (event.y - key[1]) ** 2 < \
                   self.onkey_range:
                    if self.select == (-1,-1):
                        if self.vector[value[1]][value[0]].side == \
                           self.players[self.player_index % len(self.players)]:
                            self.select = value
                        else:
                            print(chars.cannot_select)
                    elif value in self.vector[self.select[1]]\
                                             [self.select[0]].check():
                        self.do(self.select,value)
                        winner = self.check_winner()
                        if winner != self.chessboard_void:
                            index = 0
                            for player in self.players:
                                if player == winner:
                                    print(self.player_name[index] + chars.win)
                                    self.gameloop = False
                        if self.gameloop:
                            self.draw(False)
                            self.select = (-1,-1)
                            self.player_index += 1
                            print('%s%s'%(self.player_name\
                                          [self.player_index % \
                                           len(self.players)], \
                                          chars.round_belongs))
                    foundclick = True
            if not foundclick:
                self.select = (-1,-1)
            ##print('%s(%d,%d).' % ((chars.selected,) + self.select))
            if self.gameloop and self.select != (-1,-1):
                print('%s%s.' % (chars.selected,self.vector\
                                 [self.select[1]][self.select[0]].name))

    def render_all(self):
        self.renderer.clear()
        self.renderer.renderFAST(self.chessboard_floor  )
        self.renderer.renderFAST(self.chessboard_base   )
        self.renderer.renderFAST(self.chessboard_texture)
        self.renderer.renderFAST(self.chessboard_mask   )
        self.renderer.GUI.refresh()

    def decode_char(self,basex,basez,tag,side):
        ##print('decoding:%s.' % tag)
        subimg = self.charsettings.get(tag,functions.void_set.VOID)
        if subimg == functions.void_set.VOID:
            return False
        index = 0
        basex -= self.char_shift_x
        basez += self.char_shift_z
        currentx = startx = basex
        currentz = basez
        ##color = self.charcodes['-'][side - self.side_shift]
        color = self.charcodes['-']\
                [(self.player_index + side + 1) % len(self.players)]
        for char in subimg:
            if index % self.char_col == 0:
                currentx = basex
                self.render_fonts.addsurface(((currentx + self.char_size,\
                                               self.char_total_y,\
                                               currentz),\
                                              (startx,\
                                               self.char_total_y,\
                                               currentz),\
                                              (startx,\
                                               self.char_total_y,\
                                               currentz + self.char_size),\
                                              (currentx + self.char_size,\
                                               self.char_total_y,\
                                               currentz + self.char_size)),\
                                             color)
                if index // self.char_col < self.char_lin:
                    currentz -= 1
            current_color = self.charcodes[char]\
                            [(side + 1) % len(self.players)]
            if current_color != color:
                self.render_fonts.addsurface(((currentx + self.char_size,\
                                               self.char_total_y,\
                                               currentz),\
                                              (startx,\
                                               self.char_total_y,\
                                               currentz),\
                                              (startx,\
                                               self.char_total_y,\
                                               currentz + self.char_size),\
                                              (currentx + self.char_size,\
                                               self.char_total_y,\
                                               currentz + self.char_size)),\
                                             color)
                color = current_color
                startx = currentx
            currentx += 1
            index += 1
        return True
        
    def render_piece(self,ln,col,\
                     tag = functions.void_set.VOID,\
                     side = 0):
        basex = (self.chessboard_lenth - ln - 1) \
                * self.pieces_magnify_x \
                + self.pieces_add_x
        basez = col \
                * self.pieces_magnify_z \
                + self.pieces_add_z
        index = 0
        for surface in (self.pieces_top  ,\
                        self.pieces_front,\
                        self.pieces_right,\
                        self.pieces_left ,\
                        self.pieces_back ):
            addingsurface = []
            for x,y,z in surface:
                addingsurface.append((x + basex,y,z + basez))
            self.render_pieces.addsurface(addingsurface,\
                                          (self.pieces_top_color  ,\
                                           self.pieces_front_color,\
                                           self.pieces_right_color,\
                                           self.pieces_left_color ,\
                                           self.pieces_back_color)[index])
            index += 1
        self.decode_char(basex,basez,tag,side)

    def placement_admiral_alpha(self,x,y,side):
        placement = []
        if functions.check_zone((x-1,y),(3,5),(0,2)) \
           and x >= 1 \
           and self.vector[y][x-1].side != side:
            placement.append((x-1,y))
        if functions.check_zone((x+1,y),(3,5),(0,2)) \
           and x <= self.chessboard_width-2 \
           and self.vector[y][x+1].side != side:
            placement.append((x+1,y))
        if functions.check_zone((x,y-1),(3,5),(0,2)) \
           and y >= 1 \
           and self.vector[y-1][x].side != side:
            placement.append((x,y-1))
        if functions.check_zone((x,y+1),(3,5),(0,2)) \
           and y <= self.chessboard_lenth-2 \
           and self.vector[y+1][x].side != side:
            placement.append((x,y+1))
        yc = y + 1
        loop = True
        while loop:
            if not functions.check_inchessboard((x,yc),\
                                                self.chessboard_width,\
                                                self.chessboard_lenth):
                loop = False
                continue
            check_object = self.vector[yc][x]
            if check_object.side != self.chessboard_void \
               and check_object.side != side:
                if check_object.name in self.admirals:
                    placement.append((x,yc))
                loop = False
                continue
            yc += 1
        return placement

    def placement_admiral_beta(self,x,y,side):
        placement = []
        if functions.check_zone((x-1,y),(3,5),(7,9)) \
           and x >= 1 \
           and self.vector[y][x-1].side != side:
            placement.append((x-1,y))
        if functions.check_zone((x+1,y),(3,5),(7,9)) \
           and x <= self.chessboard_width-2 \
           and self.vector[y][x+1].side != side:
            placement.append((x+1,y))
        if functions.check_zone((x,y-1),(3,5),(7,9)) \
           and y >= 1 \
           and self.vector[y-1][x].side != side:
            placement.append((x,y-1))
        if functions.check_zone((x,y+1),(3,5),(7,9)) \
           and y <= self.chessboard_lenth-2 \
           and self.vector[y+1][x].side != side:
            placement.append((x,y+1))
        yc = y - 1
        loop = True
        while loop:
            if not functions.check_inchessboard((x,yc)):
                loop = False
                continue
            check_object = self.vector[yc][x]
            if check_object.side != self.chessboard_void \
               and check_object.side != side:
                if check_object.name in self.admirals:
                    placement.append((x,yc))
                loop = False
                continue
            yc -= 1
        return placement

    def placement_pawn_alpha(self,x,y,side):
        if y <= self.chessboard_river and \
           self.vector[y+1][x].side != side:
            return [(x,y+1),]
        elif y <= self.chessboard_lenth-1:
            placement = []
            if y <= self.chessboard_lenth-2:
                if self.vector[y+1][x].side != side:
                    placement.append((x,y+1))
            if x >= 1:
                if self.vector[y][x-1].side != side:
                    placement.append((x-1,y))
            if x <= self.chessboard_width-2:
                if self.vector[y][x+1].side != side:
                    placement.append((x+1,y))
            return placement
        return []

    def placement_pawn_beta(self,x,y,side):
        if y > self.chessboard_river and \
           self.vector[y-1][x].side != side:
            return [(x,y-1),]
        elif y >= 0:
            placement = []
            if y >= 1:
                if self.vector[y-1][x].side != side:
                    placement.append((x,y-1))
            if x >= 1:
                if self.vector[y][x-1].side != side:
                    placement.append((x-1,y))
            if x <= self.chessboard_width-2:
                if self.vector[y][x+1].side != side:
                    placement.append((x+1,y))
            return placement
        return []

    def placement_chariot(self,x,y,side):
        placement = []
        for xadd,yadd in ((1,0),(-1,0),(0,1),(0,-1)):
            xc = x
            yc = y
            loop = True
            while loop:
                xc += xadd
                yc += yadd
                if not functions.check_inchessboard((xc,yc)):
                    loop = False
                    continue
                if self.vector[yc][xc].side == side:
                    loop = False
                elif self.vector[yc][xc].side == self.chessboard_void:
                    placement.append((xc,yc))
                else:
                    placement.append((xc,yc))
                    loop = False
        return placement

    def placement_steed(self,x,y,side):
        placement = []
        check_index = 0
        check_dict = {0:(1,0),1:(0,-1),2:(-1,0),3:(0,1)}
        for xadd,yadd in (( 2, 1),( 2,-1),( 1,-2),(-1,-2),\
                          (-2,-1),(-2, 1),(-1, 2),( 1, 2)):
            if functions.check_inchessboard((x+xadd,y+yadd)):
                if self.vector[y+yadd][x+xadd].side != side:
                    if self.check_stumbling_horse and \
                       functions.check_inchessboard\
                       ((x + check_dict[check_index//2][0],\
                         y + check_dict[check_index//2][1]),\
                        self.chessboard_width,\
                        self.chessboard_lenth):
                        if self.vector\
                           [y + check_dict[check_index//2][1]]\
                           [x + check_dict[check_index//2][0]]\
                           .side == self.chessboard_void:
                            placement.append((x+xadd,y+yadd))
                    else:
                        placement.append((x+xadd,y+yadd))
            check_index += 1
        return placement

    def placement_vizier_alpha(self,x,y,side):
        placement = []
        for xadd,yadd in ((1,1),(1,-1),(-1,-1),(-1,1)):
            if functions.check_inchessboard((x+(xadd*2),y+(yadd*2)),\
                                            self.chessboard_width,\
                                            self.chessboard_lenth) and \
               y+(yadd*2) <= self.chessboard_river:
                if self.vector[y+(yadd*2)][x+(xadd*2)].side != side:
                    if self.check_stumbling_vizier:
                        if self.vector[y+yadd][x+xadd].side == \
                           self.chessboard_void:
                            placement.append((x+(xadd*2),y+(yadd*2)))
                    else:
                        placement.append((x+(xadd*2),y+(yadd*2)))
        return placement

    def placement_vizier_beta(self,x,y,side):
        placement = []
        for xadd,yadd in ((1,1),(1,-1),(-1,-1),(-1,1)):
            if functions.check_inchessboard((x+(xadd*2),y+(yadd*2)),\
                                            self.chessboard_width,\
                                            self.chessboard_lenth) and \
               y+(yadd*2) > self.chessboard_river:
                if self.vector[y+(yadd*2)][x+(xadd*2)].side != side:
                    if self.check_stumbling_vizier:
                        if self.vector[y+yadd][x+xadd].side == \
                           self.chessboard_void:
                            placement.append((x+(xadd*2),y+(yadd*2)))
                    else:
                        placement.append((x+(xadd*2),y+(yadd*2)))
        return placement

    def placement_chevalier_alpha(self,x,y,side):
        placement = []
        for xadd,yadd in ((1,1),(1,-1),(-1,-1),(-1,1)):
            if functions.check_zone((x+xadd,y+yadd),(3,5),(0,2)):
                if self.vector[y+yadd][x+xadd].side != side:
                    placement.append((x+xadd,y+yadd))
        return placement

    def placement_chevalier_beta(self,x,y,side):
        placement = []
        for xadd,yadd in ((1,1),(1,-1),(-1,-1),(-1,1)):
            if functions.check_zone((x+xadd,y+yadd),(3,5),(7,9)):
                if self.vector[y+yadd][x+xadd].side != side:
                    placement.append((x+xadd,y+yadd))
        return placement

    def placement_cannon(self,x,y,side):
        placement = []
        for xadd,yadd in ((1,0),(-1,0),(0,1),(0,-1)):
            xc = x
            yc = y
            loop = True
            move = True
            while loop:
                xc += xadd
                yc += yadd
                if not functions.check_inchessboard((xc,yc),\
                                                    self.chessboard_width,\
                                                    self.chessboard_lenth):
                    loop = False
                    continue
                if self.vector[yc][xc].side == self.chessboard_void and move:
                    placement.append((xc,yc))
                else:
                    if move:
                        move = False
                    else:
                        if self.vector[yc][xc].side != side and \
                           self.vector[yc][xc].side != self.chessboard_void:
                            placement.append((xc,yc))
                            loop = False
        return placement

    def summon_newchessboard(self):
        self.vector = [[pieces(name = functions.void_set.VOID,pos = (x,y)) \
                        for x in range(self.chessboard_width)]\
                        for y in range(self.chessboard_lenth)]
        self.vector[0][4] = pieces(self.admirals[0],62    ,\
                self.placement_admiral_alpha,(4,0),1)
        self.vector[3][0] = pieces(chars.pawn_alpha,1     ,\
                self.placement_pawn_alpha,(0,3),1)
        self.vector[3][2] = pieces(chars.pawn_alpha,1     ,\
                self.placement_pawn_alpha,(2,3),1)
        self.vector[3][4] = pieces(chars.pawn_alpha,1     ,\
                self.placement_pawn_alpha,(4,3),1)
        self.vector[3][6] = pieces(chars.pawn_alpha,1     ,\
                self.placement_pawn_alpha,(6,3),1)
        self.vector[3][8] = pieces(chars.pawn_alpha,1     ,\
                self.placement_pawn_alpha,(8,3),1)
        self.vector[2][1] = pieces(chars.cannon_alpha,7   ,\
                self.placement_cannon,(1,2),1)
        self.vector[2][7] = pieces(chars.cannon_alpha,7   ,\
                self.placement_cannon,(7,2),1)
        self.vector[0][0] = pieces(chars.chariot_alpha,10 ,\
                self.placement_chariot,(0,0),1)
        self.vector[0][8] = pieces(chars.chariot_alpha,10 ,\
                self.placement_chariot,(8,0),1)
        self.vector[0][1] = pieces(chars.steed_alpha,6    ,\
                self.placement_steed,(1,0),1)
        self.vector[0][7] = pieces(chars.steed_alpha,6    ,\
                self.placement_steed,(7,0),1)
        self.vector[0][2] = pieces(chars.vizier_alpha,3   ,\
                self.placement_vizier_alpha,(2,0),1)
        self.vector[0][6] = pieces(chars.vizier_alpha,3   ,\
                self.placement_vizier_alpha,(6,0),1)
        self.vector[0][3] = pieces(chars.chevalier_alpha,2,\
                self.placement_chevalier_alpha,(3,0),1)
        self.vector[0][5] = pieces(chars.chevalier_alpha,2,\
                self.placement_chevalier_alpha,(5,0),1)
        self.vector[9][4] = pieces(self.admirals[1],62    ,\
                self.placement_admiral_beta,(4,9),2)
        self.vector[6][0] = pieces(chars.pawn_beta,1      ,\
                self.placement_pawn_beta,(0,6),2)
        self.vector[6][2] = pieces(chars.pawn_beta,1      ,\
                self.placement_pawn_beta,(2,6),2)
        self.vector[6][4] = pieces(chars.pawn_beta,1      ,\
                self.placement_pawn_beta,(4,6),2)
        self.vector[6][6] = pieces(chars.pawn_beta,1      ,\
                self.placement_pawn_beta,(6,6),2)
        self.vector[6][8] = pieces(chars.pawn_beta,1      ,\
                self.placement_pawn_beta,(8,6),2)
        self.vector[7][1] = pieces(chars.cannon_beta,7    ,\
                self.placement_cannon,(1,7),2)
        self.vector[7][7] = pieces(chars.cannon_beta,7    ,\
                self.placement_cannon,(7,7),2)
        self.vector[9][0] = pieces(chars.chariot_beta,10  ,\
                self.placement_chariot,(0,9),2)
        self.vector[9][8] = pieces(chars.chariot_beta,10  ,\
                self.placement_chariot,(8,9),2)
        self.vector[9][1] = pieces(chars.steed_beta,6     ,\
                self.placement_steed,(1,9),2)
        self.vector[9][7] = pieces(chars.steed_beta,6     ,\
                self.placement_steed,(7,9),2)
        self.vector[9][2] = pieces(chars.vizier_beta,3    ,\
                self.placement_vizier_beta,(2,9),2)
        self.vector[9][6] = pieces(chars.vizier_beta,3    ,\
                self.placement_vizier_beta,(6,9),2)
        self.vector[9][3] = pieces(chars.chevalier_beta,2 ,\
                self.placement_chevalier_beta,(3,9),2)
        self.vector[9][5] = pieces(chars.chevalier_beta,2 ,\
                self.placement_chevalier_beta,(5,9),2)
        self.summon_base()

##    def summon_debuggingchessboard(self):
##        self.vector = [[pieces(name = functions.void_set.VOID,pos = (x,y)) \
##                        for x in range(self.chessboard_width)]\
##                        for y in range(self.chessboard_lenth)]
##        self.vector[9][4] = pieces(self.admirals[0],62   ,\
##                self.placement_admiral_beta,(4,9),2)
##        self.vector[6][4] = pieces(chars.pawn_beta,1     ,\
##                self.placement_pawn_beta,(4,6),2)
##        self.vector[7][1] = pieces(chars.cannon_beta,7   ,\
##                self.placement_cannon,(1,7),2)
##        self.vector[9][8] = pieces(chars.chariot_beta,10 ,\
##                self.placement_chariot,(8,9),2)
##        self.vector[9][1] = pieces(chars.steed_beta,6    ,\
##                self.placement_steed,(1,9),2)
##        self.vector[9][6] = pieces(chars.vizier_beta,3   ,\
##                self.placement_vizier_beta,(6,9),2)
##        self.vector[9][3] = pieces(chars.chevalier_beta,2,\
##                self.placement_chevalier_beta,(3,9),2)
##        self.vector[4][8] = pieces(chars.enemy,100       ,\
##                functions.void_set.voidfunc_totally,(8,4),1)
##        self.summon_base()

    def summon_base(self):
        self.base = chars.angle_left_bold                              + \
                    ((chars.line_horizontal_bold + chars.T_bold)       * \
                     (self.chessboard_width-2))                        + \
                    chars.line_horizontal_bold                         + \
                    chars.angle_right_bold                             + \
                    chars.space_half                                   + \
                    chars.line_break
        for line in range(self.chessboard_lenth-2):
            if line == self.chessboard_river:
                self.base += chars.line_vertical_bold                  + \
                             chars.space_full                          + \
                             chars.river_left                          + \
                             (chars.space_full                         * \
                              ((self.chessboard_width-2)*2 - 5))       + \
                             chars.river_right                         + \
                             chars.space_full                          + \
                             chars.line_vertical_bold                  + \
                             chars.space_half                          + \
                             chars.line_break
                char = chars.T
            elif line == self.chessboard_river - 1:
                self.base += chars.line_vertical_bold                  + \
                             chars.space_full                          + \
                             ((chars.line_vertical + chars.space_full) * \
                              (self.chessboard_width-2))               + \
                             chars.line_vertical_bold                  + \
                             chars.space_half                          + \
                             chars.line_break
                char = chars.T_reversed
            else:
                self.base += chars.line_vertical_bold                  + \
                             chars.space_full                          + \
                             ((chars.line_vertical + chars.space_full) * \
                              (self.chessboard_width-2))               + \
                             chars.line_vertical_bold                  + \
                             chars.space_half                          + \
                             chars.line_break
                char = chars.cross
            self.base += chars.T_left_halfbold0                        + \
                         chars.line_horizontal                         + \
                         ((char + chars.line_horizontal)               * \
                          (self.chessboard_width-2))                   + \
                         chars.T_right_halfbold0                       + \
                         chars.space_half                              + \
                         chars.line_break
        self.base += chars.line_vertical_bold                          + \
                     chars.space_full                                  + \
                     ((chars.line_vertical + chars.space_full)         * \
                      (self.chessboard_width-2))                       + \
                     chars.line_vertical_bold                          + \
                     chars.space_half                                  + \
                     chars.line_break
        self.base += chars.angle_left_reversed_bold                    + \
                     ((chars.line_horizontal_bold                      + \
                       chars.T_reversed_bold)                          * \
                      (self.chessboard_width-2))                       + \
                     chars.line_horizontal_bold                        + \
                     chars.angle_right_reversed_bold                   + \
                     chars.space_half                                  + \
                     chars.line_break
        xcon = self.chessboard_width//2
        ycon = self.chessboard_lenth - 2
        for ln,col,char in ((0,xcon-1,chars.slash_right)     ,\
                            (0,xcon  ,chars.slash_left )     ,\
                            (1,xcon-1,chars.slash_left )     ,\
                            (1,xcon  ,chars.slash_right)     ,\
                            (ycon-1,xcon-1,chars.slash_right),\
                            (ycon-1,xcon  ,chars.slash_left ),\
                            (ycon,  xcon-1,chars.slash_left ),\
                            (ycon,  xcon  ,chars.slash_right)):
            self.base = functions.replacechar(self.base,\
                                              char,\
                                              (self.chessboard_width*2 + 1) * \
                                              (ln*2 + 1) + \
                                              col*2 + 1)
        for ln,col in ((2,1),(2,7),(3,0),(3,2),(3,4),(3,6),(3,8),\
                       (7,1),(7,7),(6,0),(6,2),(6,4),(6,6),(6,8)):
            char = chars.cross_doubled
            if col == 0:
                char = chars.T_left_doubled2
            elif col == self.chessboard_width - 1:
                char = chars.T_right_doubled2
            self.base = functions.replacechar(self.base,\
                                         char,\
                                         ((self.chessboard_width*2+1)*2*ln) + \
                                         (col*2))

    def do(self,pos_from,pos_to):
        fromx,fromy = pos_from
        tox,toy = pos_to
        self.vector[pos_to[1]][pos_to[0]] = self.vector[fromy][fromx]
        self.vector[fromy][fromx] = pieces(name = functions.void_set.VOID,\
                                           pos = pos_from)
        self.vector[toy][tox].pos = pos_to
        self.draw(False)

    def draw(self,returnv = False):
##        base = '┏━┳━┳━┳━┳━┳━┳━┳━┓ \n' + \
##               '┃　│　│　│╲│╱│　│　│　┃ \n' + \
##               '┠─┼─┼─┼─┼─┼─┼─┼─┨ \n' + \
##               '┃　│　│　│╱│╲│　│　│　┃ \n' + \
##               '┠─╬─┼─┼─┼─┼─┼─╬─┨ \n' + \
##               '┃　│　│　│　│　│　│　│　┃ \n' + \
##               '╠─┼─╬─┼─╬─┼─╬─┼─╣ \n' + \
##               '┃　│　│　│　│　│　│　│　┃ \n' + \
##               '┠─┴─┴─┴─┴─┴─┴─┴─┨ \n' + \
##               '┃　楚河　　　　　　　　　汉界　┃ \n' + \
##               '┠─┬─┬─┬─┬─┬─┬─┬─┨ \n' + \
##               '┃　│　│　│　│　│　│　│　┃ \n' + \
##               '╠─┼─╬─┼─╬─┼─╬─┼─╣ \n' + \
##               '┃　│　│　│　│　│　│　│　┃ \n' + \
##               '┠─╬─┼─┼─┼─┼─┼─╬─┨ \n' + \
##               '┃　│　│　│╲│╱│　│　│　┃ \n' + \
##               '┠─┼─┼─┼─┼─┼─┼─┼─┨ \n' + \
##               '┃　│　│　│╱│╲│　│　│　┃ \n' + \
##               '┗━┻━┻━┻━┻━┻━┻━┻━┛ \n'
        if returnv:
            base = self.base
            linesize = self.chessboard_width*4 + 2
        else:
            self.onkey = {}
            self.render_pieces.clear()
            self.render_fonts .clear()
        for line in self.vector:
            for item in line:
                if item.name != functions.void_set.VOID:
                    if returnv:
                        base = functions.replacechar(base,item.name,\
                                                     (linesize*item.pos[1]) + \
                                                     (item.pos[0]*2))
                    else:
                        self.render_piece(item.pos[0],item.pos[1],\
                                          item.name,item.side)
                if not returnv:
                    basex = (self.chessboard_lenth - item.pos[0] - 1) \
                             * self.pieces_magnify_x \
                             + self.pieces_add_x
                    basez = item.pos[1] \
                            * self.pieces_magnify_z \
                            + self.pieces_add_z
                    pos2d = CHCMR7.renderer_BASE.posshift((\
                        self.center[0] + basex,\
                        self.center[1],\
                        self.center[2] + basez),\
                            self.renderer.heading[0],\
                            self.renderer.heading[1],\
                            self.renderer.magnify)
                    self.onkey[(pos2d[0] + self.window_x // 2,\
                                pos2d[1] + self.window_y // 2)] = \
                                (item.pos[0],item.pos[1])
##        count = 0
##        for item in draw_placement:
##            base = functions.replacechar(base,chars.nums_circled[count],\
##                                         (linesize*item[1]) + (item[0]*2) + 1)
##            count += 1
        ##print(self.render_fonts.length)
        if returnv:
            return base
        else:
            self.render_all()
            self.renderer.renderFAST(self.render_pieces)
            self.renderer.renderFAST(self.render_fonts)

    def check_winner(self):
        haveadmiral = [False for _ in range(len(self.players))]
        for line in self.vector:
            for item in line:
                if item.side != self.chessboard_void and \
                   item.name in self.admirals:
                    index = 0
                    for player in self.players:
                        if item.side == player:
                            haveadmiral[index] = True
                            break
                        index += 1
        playersc = []
        for player in self.players:
            playersc.append(player)
        index = 0
        for item in haveadmiral:
            if not item:
                playersc.pop(index)
                return playersc[0]
            index += 1
        return self.chessboard_void

if __name__ == '__main__':
    c = chessboard()
    c.summon_newchessboard()
    print(c.draw(True))
    c.draw(False)
    ##c.summon_debuggingchessboard()
    ##print(c.draw())
    c.check_winner()
    print('Done')
