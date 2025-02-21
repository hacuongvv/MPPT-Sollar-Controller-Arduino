#include <lvgl.h>
#include <TAMC_GT911.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TAMC_GT911 touch = TAMC_GT911(21, 22, 36, 320, 480);

lv_obj_t *container_auto, *container_manual;
lv_obj_t *lbl_coords, *lbl_status;
lv_obj_t *btn_switch, *btn_run_auto, *btn_run_manual;
lv_obj_t *speed_slider;
bool is_running = false; // Trạng thái máy CNC
bool is_auto_mode = true; // Chế độ hiện tại (Auto / Manual)

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

    lv_obj_add_flag(btn_switch, is_running ? LV_OBJ_FLAG_HIDDEN : 0);
}

void switch_mode(lv_event_t *e) {
    if (is_running) return;

    is_auto_mode = !is_auto_mode;
    lv_obj_add_flag(is_auto_mode ? container_manual : container_auto, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(is_auto_mode ? container_auto : container_manual, LV_OBJ_FLAG_HIDDEN);

    const char *mode_text = is_auto_mode ? "Manual" : "Auto";
    lv_label_set_text(lv_obj_get_child(btn_switch, 0), mode_text);
}

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
}

void create_manual_screen(lv_obj_t *parent) {
    static const char *btn_labels[] = {"X+", "X-", "Y+", "Y-", "Z+", "Z-"};
    for (int i = 0; i < 6; i++) {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, 80, 50);
        lv_obj_align(btn, LV_ALIGN_CENTER, (i % 2) * 100 - 50, (i / 2) * 60 - 60);
        lv_label_set_text(lv_label_create(btn), btn_labels[i]);
    }

    speed_slider = lv_slider_create(parent);
    lv_obj_set_width(speed_slider, 200);
    lv_obj_align(speed_slider, LV_ALIGN_BOTTOM_MID, 0, -60);

    btn_run_manual = lv_btn_create(parent);
    lv_obj_set_size(btn_run_manual, 100, 50);
    lv_obj_align(btn_run_manual, LV_ALIGN_BOTTOM_MID, -60, -20);
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

    container_auto = lv_obj_create(lv_scr_act());
    lv_obj_set_size(container_auto, 480, 320);
    create_auto_screen(container_auto);

    container_manual = lv_obj_create(lv_scr_act());
    lv_obj_set_size(container_manual, 480, 320);
    create_manual_screen(container_manual);
    lv_obj_add_flag(container_manual, LV_OBJ_FLAG_HIDDEN);

    btn_switch = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_switch, 100, 50);
    lv_obj_align(btn_switch, LV_ALIGN_BOTTOM_MID, 60, -20);
    lv_label_set_text(lv_label_create(btn_switch), "Manual");
    lv_obj_add_event_cb(btn_switch, switch_mode, LV_EVENT_CLICKED, NULL);
}

void loop() {
    lv_timer_handler();
    delay(5);
}