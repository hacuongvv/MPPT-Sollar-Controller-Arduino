#include <lvgl.h>
#include <TAMC_GT911.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#define SD_CS 5  // Chân CS của thẻ nhớ

TFT_eSPI tft = TFT_eSPI();
TAMC_GT911 touch = TAMC_GT911(21, 22, 36, 320, 480); // SCL, SDA, INT, WIDTH, HEIGHT

lv_obj_t *file_list;
String currentPath = "/";

void listFiles(const char *path);

void file_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    const char *name = lv_list_get_btn_text(file_list, btn);
    String fullPath = currentPath + "/" + name;

    if (SD.open(fullPath).isDirectory()) {
        currentPath = fullPath;
        listFiles(currentPath.c_str());
    } else if (String(name).endsWith(".gcode")) {
        Serial.printf("Chọn file: %s\n", fullPath.c_str());
    }
}

void listFiles(const char *path) {
    lv_list_clean(file_list);  // Xóa danh sách cũ

    if (strcmp(path, "/") != 0) {  // Thêm nút "Quay lại" nếu không phải thư mục gốc
        lv_obj_t *btn = lv_list_add_btn(file_list, LV_SYMBOL_LEFT, "..");
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            currentPath = currentPath.substring(0, currentPath.lastIndexOf('/'));
            if (currentPath == "") currentPath = "/";
            listFiles(currentPath.c_str());
        }, LV_EVENT_CLICKED, NULL);
    }

    File root = SD.open(path);
    while (true) {
        File file = root.openNextFile();
        if (!file) break;

        String filename = file.name();
        if (file.isDirectory()) {
            lv_obj_t *btn = lv_list_add_btn(file_list, LV_SYMBOL_DIRECTORY, filename.c_str());
            lv_obj_add_event_cb(btn, file_event_handler, LV_EVENT_CLICKED, NULL);
        } else if (filename.endsWith(".gcode")) {
            lv_obj_t *btn = lv_list_add_btn(file_list, LV_SYMBOL_FILE, filename.c_str());
            lv_obj_add_event_cb(btn, file_event_handler, LV_EVENT_CLICKED, NULL);
        }
        file.close();
    }
    root.close();
}

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);
    lv_init();

    if (!SD.begin(SD_CS)) {
        Serial.println("❌ Không tìm thấy thẻ nhớ!");
        return;
    }

    touch.begin();
    lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, LV_HOR_RES_MAX * 10);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read;
    lv_indev_drv_register(&indev_drv);

    file_list = lv_list_create(lv_scr_act());
    lv_obj_set_size(file_list, 480, 300);
    lv_obj_center(file_list);

    listFiles("/");
}

void loop() {
    lv_timer_handler();
    delay(5);
}