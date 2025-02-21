#include <lvgl.h>
#include <TAMC_GT911.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TAMC_GT911 touch = TAMC_GT911(21, 22, 36, 320, 480);

lv_obj_t *tabview;
lv_obj_t *lbl_coords, *lbl_status;
lv_obj_t *btn_run_auto, *btn_run_manual;
lv_obj_t *speed_slider;

bool is_running = false; // Trạng thái máy CNC

void update_coordinates(float x, float y, float z) {
    static char buffer[50];
    snprintf(buffer, sizeof(buffer), "X: %.2f  Y: %.2f  Z: %.2f", x, y, z);
    lv_label_set_text(lbl_coords, buffer);
}

void update_status(const char *status) {
    lv_label_set_text(lbl_status, status);
}

void toggle_run(lv_event_t *e) {
    is_running = !is_running;
    const char *status_text = is_running ? "Đang chạy" : "Sẵn sàng";
    const char *btn_text = is_running ? "Dừng" : "Chạy";

    update_status(status_text);
    lv_label_set_text(lv_obj_get_child(btn_run_auto, 0), btn_text);
    lv_label_set_text(lv_obj_get_child(btn_run_manual, 0), btn_text);

    lv_obj_add_flag(tabview, is_running ? LV_OBJ_FLAG_CLICK_FOCUSABLE : 0);
}

void speed_slider_event(lv_event_t *e) {
    int speed = lv_slider_get_value(speed_slider);
    Serial.printf("Tốc độ: %d\n", speed);
}

void manual_move_event(lv_event_t *e) {
    if (is_running) return; // Không điều khiển nếu máy đang chạy
    lv_obj_t *btn = lv_event_get_target(e);
    const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));
    Serial.printf("Điều khiển: %s\n", txt);
}

void create_auto_tab(lv_obj_t *parent) {
    lbl_coords = lv_label_create(parent);
    lv_obj_align(lbl_coords, LV_ALIGN_TOP_MID, 0, 10);
    update_coordinates(0, 0, 0);

    lbl_status = lv_label_create(parent);
    lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 40);
    update_status("Sẵn sàng");

    btn_run_auto = lv_btn_create(parent);
    lv_obj_set_size(btn_run_auto, 100, 50);
    lv_obj_align(btn_run_auto, LV_ALIGN_LEFT_MID, 30, 0);
    lv_label_set_text(lv_label_create(btn_run_auto), "Chạy");
    lv_obj_add_event_cb(btn_run_auto, toggle_run, LV_EVENT_CLICKED, NULL);
}

void create_manual_tab(lv_obj_t *parent) {
    static const char *btn_labels[] = {"X+", "X-", "Y+", "Y-", "Z+", "Z-"};
    for (int i = 0; i < 6; i++) {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, 80, 50);
        lv_obj_align(btn, LV_ALIGN_CENTER, (i % 2) * 100 - 50, (i / 2) * 60 - 60);
        lv_label_set_text(lv_label_create(btn), btn_labels[i]);
        lv_obj_add_event_cb(btn, manual_move_event, LV_EVENT_CLICKED, NULL);
    }

    btn_run_manual = lv_btn_create(parent);
    lv_obj_set_size(btn_run_manual, 100, 50);
    lv_obj_align(btn_run_manual, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_label_set_text(lv_label_create(btn_run_manual), "Chạy");
    lv_obj_add_event_cb(btn_run_manual, toggle_run, LV_EVENT_CLICKED, NULL);
}

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);
    lv_init();
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

    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);
    lv_obj_t *tab_auto = lv_tabview_add_tab(tabview, "Auto");
    lv_obj_t *tab_manual = lv_tabview_add_tab(tabview, "Manual");

    create_auto_tab(tab_auto);
    create_manual_tab(tab_manual);
}

void loop() {
    lv_timer_handler();
    delay(5);
}