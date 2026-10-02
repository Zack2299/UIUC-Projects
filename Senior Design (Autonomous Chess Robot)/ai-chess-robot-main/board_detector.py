import cv2
import numpy as np
import math
# import os
from PIL import Image
from PIL import ImageDraw
import psutil
import time
import chess
import easyocr
import os
from copy import deepcopy

# global show_cv because I didn't want to have show_cv as an input to every function
# also need skip_camera so I can use cv2.imshow if I'm using my PC
show_cv = None
show_board = None
skip_camera = None
img_size = None
enable_castling = None
def init_global(bool1, bool15, bool2, bool3, bool4):
    global show_cv
    global show_board
    global skip_camera
    global img_size
    global enable_castling
    show_cv = bool1
    show_board = bool15
    skip_camera = bool2
    img_size = bool3
    enable_castling = bool4

# had to write a custom img display function because conflict with picamera2 and opencv made it so I can't use imshow
# (I had to install headless opencv to remove conflict which removes imshow)
def display_img(img_array):
    max_windows = 5
    # if len(img_array) < max_windows: # prevent spamming windows if accidentally input an image instead of array of images (guess why this is here)
    if skip_camera: # if not running on raspi
        for i, cv2_img in enumerate(img_array):
            cv2.imshow('Image ' + str(i+1), cv2_img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    else: # if running on raspi
        for cv2_img in img_array:
            pil_img = cv2_to_pil(cv2_img)
            pil_img.show()
        input()
        for proc in psutil.process_iter():
            if proc.name() == "display":
                proc.kill()
    # else:
    #     print(f"Too many images (>{max_windows})")

def cv2_to_pil(cv2_img):
    rgb_img = cv2.cvtColor(cv2_img, cv2.COLOR_BGR2RGB)
    pil_img = Image.fromarray(rgb_img)
    return pil_img

def pil_to_cv2(pil_image):
    cv2_img = np.array(pil_image, dtype=np.uint8)
    cv2_img = cv2.cvtColor(cv2_img, cv2.COLOR_RGB2BGR)
    return cv2_img

def find_lines(img):
    gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray_img, 50, 100, apertureSize=3)

    h,w = edges.shape

    # don't detect any lines at the edges (make the edges black)
    cutoff = int(w * 0.01)
    for y in range(0,h):
        for x in range(0,w):
            if x < cutoff or x > w - cutoff or y < cutoff or y > h - cutoff:
                edges[y, x] = 0

    if (show_cv):
        display_img([edges])

    theta_thresh = 80
    horizontal_lines = cv2.HoughLines(edges, rho=1, theta=np.pi/180, threshold=60, min_theta=(theta_thresh/2-1)*np.pi/theta_thresh, max_theta=(theta_thresh/2+1)*np.pi/theta_thresh)
    vertical_lines = cv2.HoughLines(edges, rho=1, theta=np.pi/180, threshold=60, min_theta=-np.pi/theta_thresh, max_theta=np.pi/theta_thresh)

    vertical_line_points = convert_to_cartesian(vertical_lines)
    horizontal_line_points = convert_to_cartesian(horizontal_lines)

    # filter lines too close to each other
    filtered_vertical = filter_lines(vertical_line_points, int(img_size/10))
    filtered_horizontal = filter_lines(horizontal_line_points, int(img_size/10))

    # get the 9 largest lines
    sorted_vertical = sorted(filtered_vertical, key=lambda line: min(line[0][1], line[0][3]))[:9]
    sorted_horizontal = sorted(filtered_horizontal, key=lambda line: min(line[0][0], line[0][2]))[:9]

    return sorted_vertical, sorted_horizontal

def convert_to_cartesian(lines):
    # used for HoughLines
    # www.geeksforgeeks.org/line-detection-python-opencv-houghline-method/
    line_points = []
    if lines is not None:
        for line in lines:
            rho, theta = line[0]

            cos_theta  = np.cos(theta)
            sin_theta = np.sin(theta)

            x0 = cos_theta * rho
            y0 = sin_theta * rho

            x1 = int(x0 + 1000 * (-sin_theta))
            y1 = int(y0 + 1000 * (cos_theta))
            x2 = int(x0 - 1000 * (-sin_theta))
            y2 = int(y0 - 1000 * (cos_theta))

            line_points.append([[x1,y1,x2,y2]])
    return line_points

def filter_lines(lines, min_distance):
    filtered_lines = []

    # filter out lines too close to each other
    # (this assumes lines are around the same size and parallel)
    # (extremely simplified to improve computational speed because this is all we need)
    if lines is not None:
        for line1 in lines:
            x1, y1, x2, y2 = line1[0]
            line1_x_avg = (x1 + x2) / 2
            line1_y_avg = (y1 + y2) / 2
            keep_line = True
            for line2 in filtered_lines:
                x3, y3, x4, y4 = line2[0]
                
                line2_x_avg = (x3 + x4) / 2
                line2_y_avg = (y3 + y4) / 2

                # calculate dist between average points of the 2 lines
                dist = np.sqrt((line1_x_avg - line2_x_avg)**2 + (line1_y_avg - line2_y_avg)**2)

                if dist < min_distance:
                    keep_line = False
                    break

            if keep_line:
                filtered_lines.append(line1)
            
    return filtered_lines

def find_board(img):
    # https://opencvpython.blogspot.com/2012/06/sudoku-solver-part-2.html
    # https://stackoverflow.com/questions/10196198/how-to-remove-convexity-defects-in-a-sudoku-square/11366549#11366549
    # ^ two sources that were very useful for the board detection code

    vertical_lines, horizontal_lines = find_lines(img)

    # create bitmasks for vert and horiz so we can get lines and intersections
    height, width, _ = img.shape
    vertical_mask = np.zeros((height, width), dtype=np.uint8)
    horizontal_mask = np.zeros((height, width), dtype=np.uint8)

    for line in vertical_lines:
        x1, y1, x2, y2 = line[0]
        cv2.line(vertical_mask, (x1, y1), (x2, y2), (255), 2)

    for line in horizontal_lines:
        x1, y1, x2, y2 = line[0]
        cv2.line(horizontal_mask, (x1, y1), (x2, y2), (255), 2)

    # get lines and intersections of grid and corresponding contours
    intersection = cv2.bitwise_and(vertical_mask, horizontal_mask)
    board_lines = cv2.bitwise_or(vertical_mask, horizontal_mask)

    contours, hierarchy = cv2.findContours(board_lines, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    if (show_cv):
        intersections, hierarchy = cv2.findContours(intersection, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        # find midpoints of intersection contours (this gives us exact coordinates of the corners of the grid)
        intersection_points = []
        for contour in intersections:
            M = cv2.moments(contour)
            if (M["m00"] != 0):
                midpoint_x = int(M["m10"] / M["m00"])
                midpoint_y = int(M["m01"] / M["m00"])
                intersection_points.append((midpoint_x, midpoint_y))

        # sort the coordinates from left to right then top to bottom
        sorted_intersection_points = sort_square_grid_coords(intersection_points, unpacked=False)

        board_lines_img = img.copy()
        cv2.drawContours(board_lines_img, contours, -1, (255, 255, 0), 2)
        i = 0
        for points in sorted_intersection_points:
            for point in points:
                cv2.circle(board_lines_img, point, 5, (255 - (3 * i), (i % 9) * 28, 3 * i), -1)
                i += 1

        display_img([board_lines_img])

    if (len(vertical_lines) != 9 or len(horizontal_lines) != 9):
        print("Error: Grid does not match expected 9x9")
        print("# of Vertical:",len(vertical_lines))
        print("# of Horizontal:",len(horizontal_lines))
        return None, None

    # find largest contour and get rid of it because it contains weird edges from lines
    max_area = 100000 # we're assuming board is going to be big (hopefully to speed up computation on raspberry pi)
    largest = -1
    for i, contour in enumerate(contours):
        area = cv2.contourArea(contour)
        if area > max_area:
            max_area = area
            largest = i
    # "largest" is index of largest contour

    # get rid of contour containing the edges of the lines   
    contours = list(contours)
    contours.pop(largest)
    contours = tuple(contours)

    # thicken lines so that connections are made
    contour_mask = np.zeros((height, width), dtype=np.uint8)
    cv2.drawContours(contour_mask, contours, -1, (255), thickness=10)
    thick_contours, _ = cv2.findContours(contour_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # obtain largest contour of the thickened lines (the border) and approximate a 4 sided polygon onto it
    max_area = 100000
    largest = -1
    max_rect = None
    for i, contour in enumerate(thick_contours):
        area = cv2.contourArea(contour)
        if area > max_area:
            epsilon = 0.05 * cv2.arcLength(contour, True)
            rect = cv2.approxPolyDP(contour, epsilon, True) # uses Douglas-Peucker algorithm (probably overkill)
            if (len(rect) == 4):
                max_area = area
                largest = i
                max_rect = rect

    # perspective transform based on rectangle outline of board
    corners = max_rect.reshape(-1, 2) # reshapes it so each row has 2 elements
    corners = [tuple(corner) for corner in corners] # convert to tuples
    corners_sorted = sort_square_grid_coords(corners, unpacked=True)

    return corners_sorted, intersection

def warp_board(img, corners_sorted, intersection):
    height, width, _ = img.shape

    tl, tr, bl, br = corners_sorted
    src = np.float32([list(tl), list(tr), list(bl), list(br)])
    dest = np.float32([[0,0], [width, 0], [0, height], [width, height]])
    M = cv2.getPerspectiveTransform(src, dest)
    Minv = cv2.getPerspectiveTransform(dest, src)
    warped_img = img.copy()
    warped_img = cv2.warpPerspective(np.uint8(warped_img), M, (width, height))

    # perspective transform the intersections as well
    M = cv2.getPerspectiveTransform(src, dest)
    Minv = cv2.getPerspectiveTransform(dest, src)
    warped_ip = intersection.copy() # warped intersection points
    warped_ip = cv2.warpPerspective(np.uint8(warped_ip), M, (width, height))

    warped_intersections, hierarchy = cv2.findContours(warped_ip, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # find midpoints of warped intersection contours (this gives us exact coordinates of the corners of the grid)
    warped_intersection_points = []
    for contour in warped_intersections:
        M = cv2.moments(contour)
        if (M["m00"] != 0):
            midpoint_x = int(M["m10"] / M["m00"])
            midpoint_y = int(M["m01"] / M["m00"])
            warped_intersection_points.append((midpoint_x, midpoint_y))

    # sort the coordinates from left to right then top to bottom
    sorted_warped_points = sort_square_grid_coords(warped_intersection_points, unpacked=False)

    if (show_cv and False):
        contours_img = img.copy()
        cv2.drawContours(contours_img, thick_contours, -1, (0, 255, 0), 2)
        cv2.drawContours(contours_img, [thick_contours[largest]], -1, (0, 0, 255), 2)
        cv2.drawContours(contours_img, [max_rect], -1, (255, 0, 0), 2)
        for x,y in corners:
            cv2.circle(contours_img, (x, y), 5, (0, 255, 255), -1)
        display_img([contours_img])

    return warped_img, sorted_warped_points

prev_filled_contour_mask = None # probably should have stored this in game.py instead

def find_pieces(warped_img, prev_warped_img, sorted_warped_points, reader, img_idx):
    global prev_filled_contour_mask

    # flag if it is past the first iteration or not
    compare_prev_warped = img_idx > 1

    hsv_img = cv2.cvtColor(warped_img, cv2.COLOR_BGR2HSV)
    gray_img = cv2.cvtColor(warped_img, cv2.COLOR_BGR2GRAY)

    if compare_prev_warped:
        prev_hsv_img = cv2.cvtColor(prev_warped_img, cv2.COLOR_BGR2HSV)

    # threshold to find strongest colors in image
    saturation_thresh = 60
    brightness_thresh = 80
    hsv_mask_sat = cv2.inRange(hsv_img[:,:,1], saturation_thresh, 255) # saturation mask
    hsv_mask_bright = cv2.inRange(hsv_img[:,:,2], brightness_thresh, 255) # brightness mask
    hsv_mask = cv2.bitwise_and(hsv_mask_sat, hsv_mask_bright)

    if compare_prev_warped:
        prev_hsv_mask_sat = cv2.inRange(prev_hsv_img[:,:,1], saturation_thresh, 255) # saturation mask
        prev_hsv_mask_bright = cv2.inRange(prev_hsv_img[:,:,2], brightness_thresh, 255) # brightness mask
        prev_hsv_mask = cv2.bitwise_and(prev_hsv_mask_sat, prev_hsv_mask_bright)

    # apply mask on original image, and get bgr and gray version
    hsv_after = cv2.bitwise_and(hsv_img, hsv_img, mask=hsv_mask)
    bgr_after = cv2.cvtColor(hsv_after, cv2.COLOR_HSV2BGR)
    gray_after = cv2.cvtColor(bgr_after, cv2.COLOR_BGR2GRAY)

    if compare_prev_warped:
        prev_hsv_after = cv2.bitwise_and(prev_hsv_img, prev_hsv_img, mask=prev_hsv_mask)
        prev_bgr_after = cv2.cvtColor(prev_hsv_after, cv2.COLOR_HSV2BGR)
        prev_gray_after = cv2.cvtColor(prev_bgr_after, cv2.COLOR_BGR2GRAY)

    diff_mask_contours = np.zeros_like(gray_after)
    drawn_warped_img = deepcopy(warped_img)

    # if it is not the first iteration then there is a previous image. obtain difference between cur and prev img and get largest contours
    if compare_prev_warped:
        gray_after_mask = np.zeros_like(gray_after)
        prev_gray_after_mask = np.zeros_like(prev_gray_after)
        gray_after_mask[gray_after != 0] = 255
        prev_gray_after_mask[prev_gray_after != 0] = 255
        diff_mask = cv2.absdiff(gray_after_mask, prev_gray_after_mask)

        diff_contours, hierarchy = cv2.findContours(diff_mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)
        diff_contours_sorted = sorted(diff_contours, reverse=True, key=cv2.contourArea)
        if enable_castling:
            num_contours = 4 # 4 because 4 different moves for castling
        else:
            num_contours = 2 # standard move is just one piece moving from a spot. assume the 2 largest contours will be this move
        diff_contours_sorted = diff_contours_sorted[0:num_contours]
        cv2.drawContours(diff_mask_contours, diff_contours_sorted, -1, (255), thickness=cv2.FILLED)

        if show_cv:
            display_img([prev_bgr_after, bgr_after])
            display_img([diff_mask, diff_mask_contours])

    # define color thresholds to use to classify colors later on
    hue_thresh_dict = {'Red': (170,190), 'Orange':(4,24), 'Yellow': (24,44), 'Green': (50,70), 'Purple': (121,140), 
                       'Teal': (80,99), 'Pink': (140,170), 'Blue': (100,120)} # CHANGE
    
    # letter dictionary for printing move
    letter_dict = {0: 'A', 1: 'B', 2: 'C', 3: 'D', 4: 'E', 5: 'F', 6: 'G', 7: 'H'}

    if (show_cv):
        warped_img_pil = cv2_to_pil(warped_img)
        warped_img_draw = ImageDraw.Draw(warped_img_pil)

    filled_contour_mask = np.zeros_like(hsv_after)
    diff_contour_mask = np.zeros_like(hsv_after)

    pixel_thresh = 120 # was 40
    color_grid = []

    # loop through each square of chess board
    for i in range(0,8):
        color_grid.append([])
        for j in range(0,8):
            do_ocr = False

            # establish corners of current square
            tl = sorted_warped_points[i][j]
            tr = sorted_warped_points[i][j+1]
            bl = sorted_warped_points[i+1][j]
            br = sorted_warped_points[i+1][j+1]

            # straight edges around square
            x_min = min(tl[0], bl[0])
            x_max = max(tr[0], br[0])
            y_min = min(tl[1], tr[1])
            y_max = max(bl[1], br[1])

            # create mask of the square
            height, width, _ = warped_img.shape
            rect_mask = np.zeros((height, width), dtype=np.uint8)
            poly = np.array([[tl, tr, br, bl]], dtype=np.int32)
            cv2.fillPoly(rect_mask, poly, 255)

            # get contours inside square mask
            masked_hsv_after = cv2.bitwise_and(gray_after, gray_after, mask=rect_mask)
            if compare_prev_warped:
                prev_masked_hsv_after = cv2.bitwise_and(prev_gray_after, prev_gray_after, mask=rect_mask)
            # display_img([masked_hsv_after,prev_masked_hsv_after])

            contours, hierarchy = cv2.findContours(masked_hsv_after, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)
            prev_contours = None
            if compare_prev_warped:
                prev_contours, hierarchy = cv2.findContours(prev_masked_hsv_after, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)

            num_pixels = 1
            hue = 0
            avg_hue = 0
            cur_bounding_box = None
            if contours is not None or prev_contours is not None: # if contours are found in the square, find the largest one (which will be the letter if present)
                try:
                    largest_contour = max(contours, key=cv2.contourArea)
                except ValueError:
                    largest_contour = None

                prev_largest_contour = None
                if prev_contours is not None:
                    try:
                        prev_largest_contour = max(prev_contours, key=cv2.contourArea)
                    except ValueError:
                        prev_largest_contour = None
                
                # add contour to contour mask
                if largest_contour is not None or prev_largest_contour is not None:
                    if prev_largest_contour is not None:
                        cv2.drawContours(prev_filled_contour_mask, [prev_largest_contour], -1, (255, 0, 0), thickness=cv2.FILLED)
                        cur_bounding_box = cv2.boundingRect(prev_largest_contour)

                    if largest_contour is not None:
                        cv2.drawContours(filled_contour_mask, [largest_contour], -1, (255, 0, 0), thickness=cv2.FILLED)
                        cur_bounding_box = cv2.boundingRect(largest_contour)
                        
                    hue_sum = 0
                    if compare_prev_warped:
                        # obtain difference mask (which will contain letters on squares that have changed)
                        diff_contour_mask = cv2.bitwise_or(filled_contour_mask, prev_filled_contour_mask, mask=diff_mask_contours)
                        diff_contour_mask = cv2.bitwise_and(diff_contour_mask, diff_contour_mask, mask=rect_mask)

                        # diff calculation
                        diff_rect = diff_contour_mask[y_min:y_max, x_min:x_max, 0]
                        num_diff_pixels = np.count_nonzero(diff_rect)
                        
                        if num_diff_pixels > pixel_thresh: # if large enough, we want to run ocr on the cur square
                            do_ocr = True

                            cv2.drawContours(drawn_warped_img, diff_contours_sorted, -1, (0,0,255), thickness=cv2.FILLED)

                            # get avg hue
                            filled_rect = filled_contour_mask[y_min:y_max, x_min:x_max, 0]
                            hsv_rect = hsv_after[y_min:y_max, x_min:x_max, 0]
                            both_nonzero_mask = (filled_rect > 0) & (hsv_rect != 0)
                            num_pixels = np.count_nonzero(both_nonzero_mask)
                            hue_sum = np.sum(hsv_rect[both_nonzero_mask])

                            avg_hue = hue_sum / num_pixels if num_pixels != 0 else 0

                            # only save the contour if it has enough pixels, otherwise erase it
                            if largest_contour is not None and num_pixels < pixel_thresh:
                                cv2.drawContours(filled_contour_mask, [largest_contour], -1, (0, 0, 0), thickness=cv2.FILLED) # erase
                                largest_contour = None
                        else:
                            avg_hue = 0
                    else:
                        # get avg hue
                        filled_rect = filled_contour_mask[y_min:y_max, x_min:x_max, 0]
                        hsv_rect = hsv_after[y_min:y_max, x_min:x_max, 0]
                        both_nonzero_mask = (filled_rect > 0) & (hsv_rect != 0)
                        num_pixels = np.count_nonzero(both_nonzero_mask)
                        hue_sum = np.sum(hsv_rect[both_nonzero_mask])

                        avg_hue = hue_sum / num_pixels if num_pixels != 0 else 0

                        # only save the contour if it has enough pixels, otherwise erase it
                        if largest_contour is not None and num_pixels < pixel_thresh:
                            cv2.drawContours(filled_contour_mask, [largest_contour], -1, (0, 0, 0), thickness=cv2.FILLED)
                            largest_contour = None

            piece_found = False
            
            # if we should do ocr on this square, then straighten square, run ocr, rotate by 90 degrees, and run ocr again.
            # whichever of the two ocr runs has a higher accuracy will be used
            if do_ocr:
                x, y, w, h = cur_bounding_box
                border = 20 # widen bounding box by _ pixels for each letter
                bl = (max(y-border, 0),max(x-border, 0))
                tr = (min(y+h+border, img_size),min(x+w+border, img_size))

                box_img = masked_hsv_after[bl[0]:tr[0], bl[1]:tr[1]]
                center = ((tr[0]+tr[1])//2, (bl[0]+bl[1])//2)
                box_img[box_img != 0] = 255 # bitmap of img of current letter
                rows, cols = (box_img.shape[1], box_img.shape[0])

                img_to_read_copy = None
                box_contours, hierarchy = cv2.findContours(box_img, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)
                box_contours_sorted = sorted(box_contours, reverse=True, key=cv2.contourArea)
                
                if len(box_contours_sorted) != 0:
                    # straighten the letter based on its contours' bounding box
                    # https://stackoverflow.com/questions/11627362/how-to-straighten-a-rotated-rectangle-area-of-an-image-using-opencv-in-python <- this helped a lot
                    box_contour = box_contours_sorted[0]
                    drawn_box_img = cv2.cvtColor(box_img, cv2.COLOR_GRAY2BGR)
                    cv2.drawContours(drawn_box_img, [box_contour], -1, (255, 255, 0), 1)
                    if show_cv:
                        display_img([box_img])
                    rotated_bounding_box = cv2.minAreaRect(box_contour) # center (x,y), (width, height), angle of rotation 
                    center, (w, h), theta = rotated_bounding_box
                    rotated_box_points = cv2.boxPoints(rotated_bounding_box)
                    rotated_box_points = np.int0(rotated_box_points)
                    cv2.drawContours(drawn_box_img, [rotated_box_points], -1, (0, 255, 255), 1)

                    if show_cv:
                        display_img([drawn_box_img])
            
                    M = cv2.getRotationMatrix2D(center, theta, 1)
                    img_to_read = cv2.warpAffine(box_img, M, (cols, rows))
                    img_to_read_copy = img_to_read
                    img_to_read_bigger = np.zeros((cols + 40, rows + 40))
                    img_to_read_bigger = img_to_read_bigger.astype(np.uint8)

                    # put img_to_read in center of img_to_read_bigger
                    start_row = int((img_to_read_bigger.shape[0] - img_to_read.shape[0]) // 2)
                    start_col = int((img_to_read_bigger.shape[1] - img_to_read.shape[1]) // 2)
                    end_row = start_row + img_to_read.shape[0]
                    end_col = start_col + img_to_read.shape[1]
                    img_to_read_bigger[start_row:end_row, start_col:end_col] = img_to_read

                    img_to_read = deepcopy(img_to_read_bigger)
                else:
                    # print("used box_img")
                    img_to_read = box_img

                letter1, confidence1 = ("X (remove)", 0)
                letter2, confidence2 = ("X (remove)", 0)

                if show_cv:
                    display_img([img_to_read])

                # detect letter in current square (first ocr)
                result1 = reader.readtext(
                    image=img_to_read,
                    allowlist="PKQRBN", # only want these letters
                    rotation_info=[0, 180], # either rightside up or upside down
                    text_threshold=0.45,
                    low_text = 0.1,
                    min_size = 5
                )
                if len(result1) != 0:
                    _, letter1, confidence1 = result1[0]
                else:
                    letter1 = "X (remove)"
                    confidence1 = 0

                if img_to_read_copy is not None:
                    # rotate straightened letter by 90 degrees (because sometimes letter is sideways so we want a pass of the
                    # letter through the ocr either upside down or rightside up--not sideways. then get the best detection, which should be the
                    # pass that is not sideways)
                    M = cv2.getRotationMatrix2D(center, 90, 1)
                    img_to_read = cv2.warpAffine(img_to_read_copy, M, (cols, rows))
                    img_to_read_bigger = np.zeros((cols + 40, rows + 40))
                    img_to_read_bigger = img_to_read_bigger.astype(np.uint8)

                    # put img_to_read in center of img_to_read_bigger (probably should have made this section a separate function)
                    start_row = int((img_to_read_bigger.shape[0] - img_to_read.shape[0]) // 2)
                    start_col = int((img_to_read_bigger.shape[1] - img_to_read.shape[1]) // 2)
                    end_row = start_row + img_to_read.shape[0]
                    end_col = start_col + img_to_read.shape[1]
                    img_to_read_bigger[start_row:end_row, start_col:end_col] = img_to_read

                    img_to_read = deepcopy(img_to_read_bigger)
                    if show_cv:
                        display_img([img_to_read])

                    # detect letter in current square (second ocr)
                    result2 = reader.readtext(
                        image=img_to_read,
                        allowlist="PKQRBN", # only want these letters
                        rotation_info=[0, 180], # either rightside up or upside down
                        text_threshold=0.45,
                        low_text = 0.1,
                        min_size = 5
                    )

                    # compare results of the 2 ocr detections and get the best (which should be the rightside up/upside down one)
                    if len(result2) != 0:
                        _, letter2, confidence2 = result2[0]

                        if confidence1 >= confidence2:
                            _, letter, confidence = result1[0]
                        else:
                            _, letter, confidence = result2[0]
                    else:
                        letter2 = "X (remove)"
                        confidence2 = 0

                        letter, confidence = (letter1, confidence1)
                else:
                    letter, confidence = (letter1, confidence1)

                # print results and confidence of letter
                if show_cv or show_board:
                    coord = letter_dict[j] + str(8 - i)
                    print(letter, confidence, coord)
                    if show_cv:
                        display_img([img_to_read])

                for color, (lower, upper) in hue_thresh_dict.items(): # for each color threshold
                    if lower <= avg_hue < upper:
                        # update color_grid
                        color_grid[i].append([color, avg_hue, num_pixels, letter])
                        piece_found = True

                        # draw color found onto cv image
                        if show_cv:
                            y,x = tl[0] + 5, tl[1] + 5
                            warped_img_draw.text((y,x), color, fill=(255, 0, 0))
                        break
                if not piece_found:
                    color_grid[i].append([None, avg_hue, num_pixels, letter])
                # print("stopping ocr")
            else:
                color_grid[i].append([None, avg_hue, num_pixels, None])

    if show_cv and False:
        warped_img_draw = pil_to_cv2(warped_img_draw._image)

        bgr_after_intersections = bgr_after.copy()
        for points in sorted_warped_points:
                for point in points:
                    cv2.circle(bgr_after_intersections, point, 1, (255, 255, 255), -1)
        # display_img([warped_img_draw, bgr_after_intersections, filled_contour_mask])
        display_img([bgr_after_intersections, filled_contour_mask])

    # print color_grid. only print when the color is found a lot in the square (> pixel_thresh times)
    if show_cv:
        # print("|avg_hue, num_pixels, letter|")
        for row in color_grid:
            print("||", end="")
            for color, avg_hue, num_pixels, letter in row:
                if letter is not None and letter[0] == "X":
                    print(f"{int(num_pixels)},   {letter}\t|", end="")
                elif num_pixels > pixel_thresh:
                    print(f"{int(avg_hue)}, {color}, {letter}\t|", end="") # THE GOOD ONE
                    # print(f"{int(num_pixels)},   {letter}\t|", end="") # for figuring out pixel_thresh
                else:
                    print("\t\t|", end="")
            print("|")

    prev_filled_contour_mask = deepcopy(filled_contour_mask)

    return color_grid, drawn_warped_img

def sort_square_grid_coords(coordinates, unpacked):
    # this function assumes there are a perfect square amount of coordinates
    sqrt_len = int(math.sqrt(len(coordinates)))
    sorted_coords = sorted(coordinates, key=lambda coord: coord[1]) # first sort by y values
    # then group rows of the square (for example, 9x9 grid would be 81 coordinates so split into 9 arrays of 9)
    rows = [sorted_coords[i:i+sqrt_len] for i in range(0, len(sorted_coords), sqrt_len)]
    for row in rows:
        row.sort(key=lambda coord: coord[0]) # now sort each row by x

    if (unpacked == False):
        return rows
    
    collapsed = [coord for row in rows for coord in row] # unpack/collapse groups to just be an array of tuples
    return collapsed