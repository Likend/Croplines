import cv2


def selectArea(image: cv2.typing.MatLike, filer_pix_size: int = 5) -> tuple[tuple[int, int], tuple[int, int]]:
    '''
    return value: 
    ((min_x,min_y), (max_x, max_y))
    '''
    img_gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    img_bin = cv2.threshold(img_gray, thresh=127,
                            maxval=255, type=cv2.THRESH_BINARY_INV)[1]
    contours = cv2.findContours(
        img_bin, mode=cv2.RETR_EXTERNAL, method=cv2.CHAIN_APPROX_NONE)[0]

    min_x_ls = []
    min_y_ls = []
    max_x_ls = []
    max_y_ls = []
    for contour in contours:
        if cv2.contourArea(contour) >= filer_pix_size:  # 降噪
            min_x_ls.append(min([i[0][0] for i in contour]))
            min_y_ls.append(min([i[0][1] for i in contour]))
            max_x_ls.append(max([i[0][0] for i in contour]))
            max_y_ls.append(max([i[0][1] for i in contour]))

    min_x = min(min_x_ls)
    min_y = min(min_y_ls)
    max_x = max(max_x_ls)
    max_y = max(max_y_ls)
    return ((min_x, min_y), (max_x, max_y))
