import cv2
from imgalgor import selectArea

image = cv2.imread("test.tif", cv2.IMREAD_COLOR)

cv2.imshow("img", image)
cv2.waitKey(0)

((min_x, min_y), (max_x, max_y)) = selectArea(image, 2, 5)

cv2.rectangle(image, (min_x, min_y), (max_x, max_y), (255, 0, 0), 2)
cv2.imshow("img", image)
cv2.waitKey(0)

cv2.imwrite("0008_out.tif", image)

1+1
