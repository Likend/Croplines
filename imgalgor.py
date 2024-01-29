import cv2


def selectArea(image: cv2.typing.MatLike, filer_pix_size: int = 5, expand_size: int = 0) -> tuple[tuple[int, int], tuple[int, int]]:
    '''
    # para:
    filer_pix_size: 忽略黑像素的大小
    expand_size: 留边空白大小

    # return: 
    ((left ,top), (right, botton))
    '''
    height, width = image.shape[0:2]
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

    l = min_x-expand_size
    r = max_x+expand_size
    t = min_y-expand_size
    b = max_y+expand_size

    if l < 0:
        l = 0
    if t < 0:
        t = 0
    if r > width:
        r = width
    if b > height:
        b = height
    return ((l, t), (r, b))
