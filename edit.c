void home_all_event(lv_event_t *e) {
    Serial.println("Home tất cả trục");
    update_status("Đang về Home...");
}

void home_axis_event(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));
    Serial.printf("Home trục: %s\n", txt);
}

// Cập nhật lại giao diện Auto (Thêm nút Home tổng)
void create_auto_screen(lv_obj_t *parent) {
    lbl_coords = lv_label_create(parent);
    lv_obj_align(lbl_coords, LV_ALIGN_TOP_MID, 0, 10);
    update_coordinates(0, 0, 0);

    lbl_status = lv_label_create(parent);
    lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 40);
    update_status("Sẵn sàng");

    btn_run_auto = lv_btn_create(parent);
    lv_obj_set_size(btn_run_auto, 100, 50);
    lv_obj_align(btn_run_auto, LV_ALIGN_BOTTOM_MID, -60, -20);
    lv_label_set_text(lv_label_create(btn_run_auto), "Chạy");
    lv_obj_add_event_cb(btn_run_auto, toggle_run, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_home = lv_btn_create(parent);
    lv_obj_set_size(btn_home, 100, 50);
    lv_obj_align(btn_home, LV_ALIGN_BOTTOM_MID, 60, -20);
    lv_label_set_text(lv_label_create(btn_home), "Home");
    lv_obj_add_event_cb(btn_home, home_all_event, LV_EVENT_CLICKED, NULL);
}

// Cập nhật lại giao diện Manual (Thêm nút Home từng trục)
void create_manual_screen(lv_obj_t *parent) {
    static const char *btn_labels[] = {"X+", "X-", "Y+", "Y-", "Z+", "Z-"};
    for (int i = 0; i < 6; i++) {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, 80, 50);
        lv_obj_align(btn, LV_ALIGN_CENTER, (i % 2) * 100 - 50, (i / 2) * 60 - 60);
        lv_label_set_text(lv_label_create(btn), btn_labels[i]);
    }

    static const char *home_labels[] = {"Home X", "Home Y", "Home Z"};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, 100, 50);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, (i - 1) * 120, -20);
        lv_label_set_text(lv_label_create(btn), home_labels[i]);
        lv_obj_add_event_cb(btn, home_axis_event, LV_EVENT_CLICKED, NULL);
    }
}