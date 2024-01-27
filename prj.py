import os
import re
import pickle
from typing import TypedDict, Any
from collections.abc import Callable
import cv2
import numpy as np


def cv_imread(filepath: str):
    '''解决opencv不能读取中文的bug'''
    cv_img = cv2.imdecode(np.fromfile(
        filepath, dtype=np.uint8), cv2.IMREAD_COLOR)
    return cv_img


def scan_files(directory, prefix=None, postfix=None):
    '''扫描路径全部文件'''
    files_list = []
    for root, sub_dirs, files in os.walk(directory):
        sub_dirs[:] = []  # 忽略子文件夹
        for special_file in files:
            if postfix:
                if special_file.endswith(postfix):
                    files_list.append(os.path.join(root, special_file))
            elif prefix:
                if special_file.startswith(prefix):
                    files_list.append(os.path.join(root, special_file))
            else:
                files_list.append(os.path.join(root, special_file))

    return files_list


def natural_sort_key(s):
    # 将字符串按照数字和非数字部分分割，返回分割后的子串列表
    sub_strings = re.split(r'(\d+)', s)
    # 如果当前子串由数字组成，则将它转换为整数；否则返回原始子串
    sub_strings = [int(c) if c.isdigit() else c for c in sub_strings]
    # 根据分割后的子串列表以及上述函数的返回值，创建一个新的列表
    # 按照数字部分从小到大排序，然后按照非数字部分的字典序排序
    return sub_strings


class PrjNoImgFileError(Exception):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)


class PrjFileFormatError(Exception):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)


class PrjData(TypedDict):
    total_num: int
    img_files: list[str]
    line_lists: list[list[float]]


class Prj:
    def __init__(self) -> None:
        self.data: PrjData = {}
        self.dir: str = ''
        self.is_change = False
        self.curr_page: int = -1

        self.pic = None

        self.output_folder: str = "out"

    def loadImgFromDir(self, prj_dir: str) -> None:
        self.dir = prj_dir
        try:
            self.load()
        except FileNotFoundError:
            self.data['img_files'] = scan_files(prj_dir, postfix=tuple(
                ['jpg', 'jpeg', 'bmp', 'png', 'tif', 'tiff', 'gif']))
            if len(self.data['img_files']) == 0:
                raise PrjNoImgFileError
            self.total_num = len(self.data['img_files'])
            self.line_lists = [[] for _ in range(self.total_num)]
            self.is_change = True
            self.imgLsNaturalSort()

    def __len__(self) -> int:
        return len(self.data["img_files"])

    def imgLsNaturalSort(self) -> None:
        """
        按文件名的结构排序，即依次比较文件名的非数字和数字部分
        """
        self.img_files = sorted(
            self.img_files, key=natural_sort_key)

    @property
    def img_files(self) -> list[str]:
        return self.data['img_files']

    @img_files.setter
    def img_files(self, value: list):
        self.data["img_files"] = value

    @property
    def total_num(self) -> int:
        return self.data['total_num']

    @total_num.setter
    def total_num(self, value: int):
        self.data['total_num'] = value

    @property
    def line_lists(self) -> list[list[int]]:
        return self.data['line_lists']

    @line_lists.setter
    def line_lists(self, value: list[list[int]]):
        self.data['line_lists'] = value

    def getLineList(self, index: int) -> list[int]:
        return self.line_lists[index]

    @property
    def img_filenames(self) -> list:
        return [os.path.basename(i) for i in self.data["img_files"]]

    @property
    def output_dir(self) -> str:
        return f'{self.dir}\\{self.output_folder}'

    def add_line_data(self, page_index: int, line_data: int):
        '''将line_data数据添加到data["line_lists"][page_index]中
        line_data为线在图像上的纵坐标（以左上角为远点，图片像素为单位）
        '''
        self.data["line_lists"][page_index].append(line_data)
        self.is_change = True

    def save(self):
        with open(f'{self.dir}/croplines.cpln', 'wb') as file:
            pickle.dump(obj=self.data, file=file, protocol=True)
        self.is_change = False

    def load(self):
        '''有可能产生异常 FileNotFoundError'''
        with open(f'{self.dir}/croplines.cpln', 'rb') as file:
            data: PrjData = pickle.load(file)
        self.is_change = False

        # check file
        if "total_num" not in data:
            raise PrjFileFormatError
        if "img_files" not in data:
            raise PrjFileFormatError
        if "line_lists" not in data:
            raise PrjFileFormatError
        if len(data['img_files']) != data['total_num']:
            raise PrjFileFormatError
        if len(data['line_lists']) != data['total_num']:
            raise PrjFileFormatError
        self.data = data

    def __getPageNotCurr(self, index: int):
        '''仅获取图像数据'''
        try:
            return cv_imread(self.img_files[index])
        except FileExistsError:
            return None

    def getPage(self, index: int):
        '''将index页设为当前页，获取该页的图像数据'''
        self.curr_page = index
        self.pic = self.__getPageNotCurr(index)
        return self.pic

    def cropPage(self, index, finish_callback: Callable[[], Any] = None):
        '''裁剪某一页'''
        if self.pic is None:
            return
        # 避免重复加载
        if index == self.curr_page:
            pic = self.pic
        else:
            pic = self.__getPageNotCurr(index)

        if not os.path.exists(self.output_dir):  # 避免报错
            os.makedirs(self.output_dir)

        crop_lines = self.getLineList(index) + [pic.shape[0]]
        crop_lines.sort()
        line_prev = 0
        cout = 1
        for line_curr in crop_lines:
            pic_crop = pic[line_prev:line_curr, :]
            # cv2.imshow("pic_crop", pic_crop)
            # cv2.waitKey(0)
            # pic_crop.tofile(
            # f'{self.dir}\\{index+1}-{cout}.png')
            cv2.imwrite(f"{self.output_dir}\\{index+1}-{cout}.png", pic_crop)
            line_prev = line_curr
            cout += 1

        if finish_callback is not None:
            finish_callback()

    def cropAllPage(self,
                    finish_callback: Callable[[], Any] = None,
                    processing_callback: Callable[[int], Any] = None):
        for i in range(self.total_num):
            if processing_callback is not None:
                processing_callback(i)
            self.cropPage(i)

        if finish_callback is not None:
            finish_callback()
