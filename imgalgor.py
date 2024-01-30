import cv2


def selectArea(image: cv2.typing.MatLike, filer_pix_size: int = 8, expand_size: int = 0) -> tuple[tuple[int, int], tuple[int, int]]:
    '''
    自动选择黑色像素区域
    # para:
    filer_pix_size: 忽略黑像素的大小
    expand_size: 留边空白大小

    # return: 
    ((left ,top), (right, botton)).

    If the image is empty, return ((-1, -1), (-1, -1)).
    '''
    height, width = image.shape[0:2]
    if height == 0 or width == 0:
        return ((-1, -1), (-1, -1))
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
            xs = [i[0][0] for i in contour]
            if 0 in xs or width-1 in xs:
                continue
            ys = [i[0][1] for i in contour]
            if 0 in ys or height-1 in ys:
                continue
            min_x_ls.append(min(xs))
            min_y_ls.append(min(ys))
            max_x_ls.append(max(xs))
            max_y_ls.append(max(ys))

    if len(min_x_ls) == 0 or len(min_y_ls) == 0 or len(max_x_ls) == 0 or len(max_y_ls) == 0:
        return ((-1, -1), (-1, -1))

    min_x = min(min_x_ls)
    min_y = min(min_y_ls)
    max_x = max(max_x_ls)
    max_y = max(max_y_ls)

    l = min_x - expand_size
    r = max_x + expand_size
    t = min_y - expand_size
    b = max_y + expand_size

    if l < 0:
        l = 0
    if t < 0:
        t = 0
    if r > width:
        r = width
    if b > height:
        b = height
    return ((l, t), (r, b))
