bool sd_initialized = false;

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);
    lv_init();

    // Kiểm tra và khởi tạo thẻ nhớ
    if (!SD.begin(SD_CS)) {
        Serial.println("❌ Không tìm thấy thẻ nhớ!");
        sd_initialized = false;
    } else {
        sd_initialized = true;
    }

    touch.begin();
    
    // Khởi tạo màn hình LVGL
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

    if (sd_initialized) {
        listFiles("/");
    } else {
        lv_obj_t *label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "Không tìm thấy thẻ nhớ!");
        lv_obj_center(label);
    }
}

void listFiles(const char *path) {
    if (!sd_initialized) return; // Kiểm tra nếu thẻ nhớ chưa được khởi tạo thì không chạy tiếp

    lv_obj_t *child;
    while ((child = lv_obj_get_child(file_list, 0)) != NULL) {
        lv_obj_del(child);
    }

    
    if (strcmp(path, "/") != 0) {
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