try:
    import cv2
except ImportError:
    cv2 = None
from .TaskUtils import project_root_path

template_folder = project_root_path / 'resource' / 'template'
std_width: int = 1280
std_height: int = 720


def is_gui_available():
    return cv2 is not None


def _check_gui():
    if not is_gui_available():
        raise ImportError("cv2 is not available, please install opencv-python.")


def set_std_width_height(width: int, height: int):
    global std_width, std_height
    std_width = width
    std_height = height


def resize_image(origin_image, width=None, height=None):
    if width is None and height is None:
        width, height = std_width, std_height
    cur_ratio = origin_image.shape[1] / origin_image.shape[0]
    if cur_ratio >= width / height:
        dsize_width = int(cur_ratio * height)
        dsize_height = height
    else:
        dsize_width = width
        dsize_height = int(width / cur_ratio)
    return cv2.resize(origin_image, (dsize_width, dsize_height), interpolation=cv2.INTER_AREA)


def show_template(template_name: str):
    _check_gui()
    template_path = template_folder / template_name
    template = cv2.imread(template_path)
    template = resize_image(template)
    cv2.namedWindow(template_name, cv2.WINDOW_NORMAL)
    cv2.imshow(template_name, template)
    print("Press any key to close the window.")
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    # 防止生成的窗口无法关闭
    cv2.waitKey(1)


def show_roi_on_template(template_name: str, roi):
    _check_gui()
    template_name = template_folder / template_name
    template = cv2.imread(template_name)
    template = resize_image(template)
    cv2.rectangle(template, (roi[0], roi[1]), (roi[2], roi[3]), (0, 255, 0), 2)
    cv2.imshow(template_name, template)
    print("Press any key to close the window.")
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    # 防止生成的窗口无法关闭
    cv2.waitKey(1)


_begin_point = None
_end_point = None
_is_cropping = False


def click_and_crop(template_name: str):
    _check_gui()
    global _begin_point, _end_point, _is_cropping

    def _click_and_crop(event, x, y, _, __):
        global _begin_point, _end_point, _is_cropping
        if event == cv2.EVENT_LBUTTONDOWN:
            _begin_point = (x, y)
            image_copy = image.copy()
            cv2.imshow(template_name, image_copy)
            _is_cropping = True
        elif event == cv2.EVENT_LBUTTONUP:
            _end_point = (x, y)
            image_copy = image.copy()
            cv2.rectangle(image_copy, _begin_point, _end_point, (0, 255, 0), 2)
            cv2.imshow(template_name, image_copy)
            _is_cropping = False
        elif _is_cropping:
            image_copy = image.copy()
            cv2.rectangle(image_copy, _begin_point, (x, y), (0, 255, 0), 2)
            cv2.imshow(template_name, image_copy)

    print("Drag mouse to select ROI, press 'S' to save, press 'Q' to quit.")
    template_path = template_folder / template_name
    image = cv2.imread(template_path)
    image = resize_image(image)
    cv2.namedWindow(template_name)
    cv2.setMouseCallback(template_name, _click_and_crop)
    while True:
        cv2.imshow(template_name, image)
        key = cv2.waitKey(0) & 0xFF
        if key == ord("s"):
            break
        elif key == ord("q"):
            return
