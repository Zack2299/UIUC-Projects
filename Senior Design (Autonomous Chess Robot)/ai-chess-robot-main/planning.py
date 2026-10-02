# UPDATE THIS WHEN JOSH SENDS FEN AND BEST MOVE 
# from chess_engine import fen_notation, best_move
import numpy as np
import heapq
import serial
from chess_ai import chess_AI, cheat_check
import chess
import time

'''
    When I get input:
    input[0] = from
    input[1] = to
    input[2] = fen_string
    input[3] = ?
    input[4] = ?
    input[5] = ?
    input[6] = ?
'''
class Path_planning():
    def __init__(self, board_width):
        self.board_width = board_width
        self.box_width = board_width / 8
        self.column_letters = np.array(['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'])
        self.row_numbers = np.array(['8', '7', '6', '5', '4', '3', '2', '1'])
        self.walls_array = np.zeros((17, 17))


    def filling_board(self, fen_string):
        fen_rows = fen_string.split("/")
        colNum = 1
        rowNum = 1
        # this turns the FEN string into the board state by saying where the piece can and cannot go
        for row in fen_rows:
            for character in row:
                # print(character)
                if(character == ' '):
                    break
                try: 
                    offset = int(character)
                    colNum += 2*offset
                        
                except ValueError:
                    self.walls_array[rowNum, colNum] = 1
                    colNum += 2 # same reasoning as above
                    # print("letter: ", character)
            colNum = 1     
            rowNum += 2
        # print(self.walls_array)

    def A_star_traversal(self, move_to_execute, capture):
        col_from = 0
        col_to = 0
        row_to = 0
        row_from = 0
        # move_to_execute = example_best_move
        for move in enumerate(move_to_execute):
            print(move)
            letter_num = move[1].split(" ")
            if(move[0] == 0):
                col_from = (np.where(self.column_letters == letter_num[0])[0][0] * 2) + 1 # All multiplied by 2 to get the actual box index in wall array
                row_from = (np.where(self.row_numbers == letter_num[1])[0][0] * 2) + 1
            else:
                col_to = (np.where(self.column_letters == letter_num[0])[0][0] * 2) + 1
                row_to = (np.where(self.row_numbers == letter_num[1])[0][0] * 2) + 1
                if(capture):
                    row_to -= 1 # We want the top left corner of board
                    col_to -= 1
        print(row_from, col_from, row_to, col_to )
        # print(self.walls_array)

        #start of traversal
        frontier = []
        heapq.heapify(frontier)
        path = []
        explored = []
        visited = []
        heapq.heapify(explored)
        cellParent = dict()
        start = (row_from, col_from)
        goal = (row_to, col_to)
        end = ()
        g = 0
        h=0
        f=0
        neighbors = []
        heapq.heappush(frontier, (0, 0, start, None)) # f val, g val, coords, parent coords

        while(len(frontier) != 0):
            curCell = frontier[0]
            if(curCell[2] in visited):
                heapq.heappop(frontier)
            else:
                heapq.heappush(explored, curCell)
                visited.append(curCell[2])
                heapq.heappop(frontier)
                # print("Current Cell: ", curCell)
                if(curCell[2] == goal):
                    end = curCell
                    break
                # neighbors = maze.neighbors(curCell[2][0], curCell[2][1])
                for rowOffset in range(-1, 2):
                    for colOffset in range(-1, 2):
                        if((rowOffset == -1 or rowOffset == 1) and colOffset != 0):
                            continue
                        if(self.walls_array[(curCell[2][0] + rowOffset)][(curCell[2][1] + colOffset)] == 0 or (curCell[2][0]+rowOffset == goal[0] and curCell[2][1]+colOffset == goal[1])): #If there is no wall from the cell we're at and were we could go in a square around the cell, add it as a neighbor
                            neighbors.append((curCell[2][0]+rowOffset, curCell[2][1]+colOffset))
                # print(neighbors)
                for cell in neighbors:
                    if(cell not in visited):
                        g = curCell[1] + 1
                        numWalls = 0
                        for rowOffset in range(-1, 2):
                            for colOffset in range(-1, 2):
                                if(((rowOffset == -1 or rowOffset == 1) and colOffset != 0) or (colOffset == 0 and rowOffset == 0) or (curCell[2] == (cell[0]+rowOffset, cell[1]+colOffset))):
                                    continue
                                if(self.walls_array[(cell[0] + rowOffset)][(cell[1] + colOffset)] == 1): #If there is no wall from the cell we're at and were we could go in a square around the cell, add it as a neighbor
                                    numWalls += 1
                        h = abs(goal[0] - cell[0]) + abs(goal[1] - cell[1]) + numWalls
                        f = g + h
                        # print(cell, f)
                        heapq.heappush(frontier, (f, g, cell, curCell[2]))
                        cellParent[cell] = curCell
                # print(frontier)
                neighbors = []

        path.append(end[2])
        parentCell = end[3]
        # print(explored)
        while(parentCell != None):
            path.append(parentCell)
            # print(path)
            for x in explored:
                # print(x, parentCell)
                if(x[2] == parentCell):
                    parentCell = x[3]
                    break
        path.reverse()
        print(path)
        return path
    
    def serialSend(self, message, cheating, promotion, castling):
        ser = serial.Serial("/dev/serial/by-id/usb-Silicon_Labs_CP2102N_USB_to_UART_Bridge_Controller_3ca0a1e14c55ed11b2b78f131d62bc44-if00-port0", 9600)  # open first serial port CHANGE ON RASPBERRY PI
        ser1 = serial.Serial("/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5XK3RJT-if00-port0", 19200)
        # print(ser.portstr)       # check which port was really used
        done = False
        if promotion:
            message += '^'
            message += ';'
        if not cheating:
            message += '>'
        print(message)
        # temp = "hello\n"
        ser.write(message.encode())      # write a string
        ser1.write(message.encode())
        if(not cheating):
            while(not done):
                retVal = ser.readline(4)
                print(retVal)
                if(retVal == b'Done' or retVal == 4):
                    done = True

        # line = ser.readline()
        # print(line)
        ser.close()
        ser1.close()
        return
                
    def pathToSerial(self, path, capLength):
        serialMessage = ""
        counter = 0
        for y,x in path:
            serialMessage += str(x)
            serialMessage += ';'
            serialMessage += str(y)
            serialMessage += ';'
            counter += 1
            if(counter == capLength):
                serialMessage += '<'
                serialMessage += ';'
        print(serialMessage)
        return serialMessage
    
    def sendCheating(self, is_cheating):
        if is_cheating:
            self.serialSend('?', 1, 0, 0)
        else:
            self.serialSend('@', 1, 0, 0)
    
    def chessExecuteMove(self, AI_output):
        print(AI_output)
        captureLocation = "a 8"
        example_fen = AI_output[3]
        example_best_move = AI_output[0]
        if len(AI_output[1]) == 2:
            example_capture = AI_output[1]
        elif len(AI_output[1]) == 1:
            example_capture = AI_output[1]
            example_capture.append(captureLocation)
            print(example_capture)
        else:
            example_capture = ''
        cheating = AI_output[2][5]
        castling = AI_output[2][2]
        capture = AI_output[2][3]
        enPasant = AI_output[2][1]
        promotion = AI_output[2][4]
        capLength = -1
        # if cheating:
        #     self.serialSend('?', cheating, promotion, castling)
        # else:
        self.filling_board(example_fen)
        if(capture or castling or enPasant):
            capPath = self.A_star_traversal(example_capture, capture)
            capLength = len(capPath)
            if(capture):
                capture = False
            if(castling):
                captureString = example_capture[0] + " " + example_capture[1]
                print(captureString)
                captureString = captureString.replace(" ", "")
                print(captureString)
                temp_board = chess.Board("r3k2K/1b1p1p2/p3p1q1/6p1/2P5/P3P3/1P2B3/R1R1Q3 b q - 7 31")
                temp_board.push(chess.Move.from_uci(captureString))
                new_FEN_string = temp_board.fen()
                self.filling_board(new_FEN_string)
            piecePath = self.A_star_traversal(example_best_move, capture)
            capPath.extend(piecePath)
            self.serialSend(self.pathToSerial(capPath, capLength), cheating, promotion, castling)
    #         # try to delay motors
    #         # time.sleep(5)
        else:
            piecePath = self.A_star_traversal(example_best_move, capture)
            self.serialSend(self.pathToSerial(piecePath, capLength), cheating, promotion, castling)






def main():
    # AI_output = chess_AI("r1bq1r2/pp2n3/4N2k/3pPppP/1b1n2Q1/2N5/PP3PP1/R1B1K2R w KQ g6 0 15", 0, 0)[0] #enpasaent possible
    AI_output = chess_AI("r3k2K/1b1p1p2/p3p1q1/6p1/2P5/P3P3/1P2B3/R1R1Q3 b q - 0 1", "", 0)[0] # castle possible
    # cheatingFlag = cheat_check("r3k2K/1b1p1p2/p3p1q1/6p1/2P5/P3P3/1P2B3/R1R1Q3 b q - 7 31","r3k2K/1b1p1p2/p3p1q1/6p1/2P5/P3P3/1P2B3/R1R1Q3 b q - 7 31")

    mychess = Path_planning(23)
    mychess.chessExecuteMove(AI_output) 
    
    
    

if __name__ == '__main__':
    main()