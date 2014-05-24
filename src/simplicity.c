#include "lylunar.h"
#include "pebble.h"

static Window *window;
//static TextLayer *text_date_layer;
static TextLayer *text_time_layer;  //时间
static TextLayer *text_sec_layer;
static TextLayer *text_nldate_layer; //农历
static TextLayer *ext_date_layer;  //阳历
static TextLayer *text_battery_layer; //电池

static Layer *battery_layer;
static Layer *line_layer;
static BitmapLayer *btimap_bt_layer;


static GBitmap *icon_bt;

static int battery_count = 0;


static void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void battery_layer_update_callback(Layer *me, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);

  //graphics_draw_rect(ctx, GRect(7, 0, 25, 13));
  graphics_draw_rect(ctx, GRect(8, 1, 18, 11));
  graphics_fill_rect(ctx, GRect(6, 3.5, 2, 7), 0, GCornerNone);

  if (battery_count == -1) {
    graphics_draw_line(ctx, GPoint(11, 9), GPoint(17, 3));
    graphics_draw_line(ctx, GPoint(17, 3), GPoint(17, 8));
    graphics_draw_line(ctx, GPoint(17, 9), GPoint(23, 3));
    return;
  }

  for (int i = 0; i < battery_count; i++) {
    graphics_fill_rect(ctx, GRect(22-3*i, 3, 2, 7), 0, GCornerNone);
  }
  //static char battery_text[] = "100";
  //snprintf(battery_text, sizeof(battery_text), "+%d", battery_count);
}

#define zhLen 3
#define MvChar(zhi) memcpy(text + place*zhLen, zhi, zhLen)
void CDateDisplayZh(Date *d, char* text)
{
  static char ZhDigit[10][zhLen+1] = { "正", "一", "二", "三", "四", "五", "六", "七", "八", "九" };
  static char ZhDigit2[3][zhLen+1] = { "初", "十", "廿" }; //digit in ten's place
  static char ZhLeap[] = "闰";
  static char ZhMonth[] = "月";
  static char* ZhTen[3] = { ZhDigit2[0], ZhDigit[2], ZhDigit[3] }; //初, 二, 三

  int i,j;
  int place = 0;
  i = d->leap?1:0;
  j = (d->month-1)/10==0 ? 0 : 1;

  if(i) MvChar(ZhLeap); //閏

  place += i;
  if(j) MvChar(ZhDigit2[1]);    //十 of 十某月

  place += j;
  if(d->month==1) MvChar(ZhDigit[0]);   //正 of 正月
  else if(d->month==10) MvChar(ZhDigit2[1]);    //十 of 十月
  else      MvChar(ZhDigit[d->month%10]); //某 of 十某月 or 某月

  place++;

  MvChar(ZhMonth);  //月
  place++;

  if(d->day%10==0){
  MvChar(ZhTen[d->day/10 - 1]); //某 of 某十日
  place++;
  MvChar(ZhDigit2[1]);    //十 of 某十日
  place++;
  }
  else{
  MvChar(ZhDigit2[d->day/10]);  //某 of 某甲日
  place++;
  MvChar(ZhDigit[d->day%10]); //甲 of 某甲日
  place++;
  }

  text[place*zhLen] = 0;

  text_layer_set_text(text_nldate_layer, text);
  
}

static void _cdate_upd(struct tm *tick_time) 
{
  Date today;


  // static char zdata2[30] ="";

  // static char year[] = "0000";
  // static char month[] = "00";
  // static char day[] = "00";
  // static char hour[] = "00";
  // strftime(year, sizeof(year), "%Y", tick_time);
  // strftime(month, sizeof(month), "%m", tick_time);
  // strftime(day, sizeof(day), "%d", tick_time);
  // strftime(hour, sizeof(hour), "%H", tick_time);

  // today.year  = 1900;
  // today.month = 11;
  // today.day = 11;
  // today.hour = 12;
  today.year = tick_time->tm_year + 1900;
  today.month = tick_time->tm_mon + 1;
  today.day   = tick_time->tm_mday;
  today.hour  = tick_time->tm_hour;

  Solar2Lunar(&today);

  CDateDisplayZh(&today, "1");
}

// bool _cdate_upd_cri(PebbleTickEvent* evt)
// {
//     return evt->units_changed & HOUR_UNIT
//       && evt->tick_time->tm_hour==23;
// }


//set data and week 
void handle_data_tick(struct tm *tick_time) {

  static char zdata[30] ="";

  static char mon_zh[] = "月";
  static char day_zh[] = "日";
  static char wday_zh[] = "日一二三四五六";
  static char par[] = "()";

  static char mon_n[] = "00";
  int m = sizeof(mon_n)-1;
  static char day_n[] = "00";
  int d = sizeof(day_n)-1;
  const int zhl = 3;

  int place = 0;

  static char wday_n[] = "0";
  int index_w = 0;

  // //strftime(sec_text, sizeof(sec_text), "%S", tick_time);
  //%u 每周的第几天，星期一为第一天 （值从1到7，星期一为1）
  strftime(mon_n, m+1, "%m", tick_time);
  strftime(day_n, d+1, "%d", tick_time);
  strftime(wday_n, 2, "%w", tick_time);

  index_w = wday_n[0] - '0';
  // strcat(zdata, mon_n);

  // strcat(zdata, mon_zh);

  memcpy(zdata + place , mon_n, m);
  place += m;
  memcpy(zdata + place , mon_zh, zhl);
  place += zhl;
  memcpy(zdata +place, day_n, d);
  place += d;
  memcpy(zdata+place, day_zh, zhl);
  place += zhl;
  memcpy(zdata +place, par, 1);
  place += 1;
  memcpy(zdata + place, wday_zh+index_w*zhl, zhl);
  place += zhl;
  memcpy(zdata+place, par+1, 1);
  place += 1;
  memcpy(zdata+place, "\0", 1);

  text_layer_set_text(ext_date_layer, zdata);

}

static void handle_sec_tick(struct tm *tick_time, TimeUnits units_changed) {

  static char sec_text[] = "00";
  strftime(sec_text, sizeof(sec_text), "%S", tick_time);
  //"%S"
  text_layer_set_text(text_sec_layer, sec_text);

}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;


  // TODO: Only update the date when it's changed.
  //strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  //text_layer_set_text(text_date_layer, date_text);

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);

  //update second  //"%S"
  static char sec_text[] = "00";
  strftime(sec_text, sizeof(sec_text), "%S", tick_time);
  text_layer_set_text(text_sec_layer, sec_text);

  handle_data_tick(tick_time);

  _cdate_upd(tick_time);

}

static void handle_bluetooth(bool connected) {


  if (connected) {
        bitmap_layer_set_bitmap(btimap_bt_layer, icon_bt);
    } else {
       // bitmap_layer_set_bitmap(btimap_bt_layer, img_bt_disconnect);
       // vibes_long_pulse();
    }
  // if (connected)
  // {
  //    bitmap_layer_set_bitmap(btimap_bt_layer, icon_bt);
  // }
  //if (connected)
    //graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  //else
    //graphics_context_set_compositing_mode(ctx, GCompOpClear);
  //graphics_draw_bitmap_in_rect(ctx, icon_bt, GRect(0, 0, 9, 12));
  //text_layer_set_text(connection_layer, connected ? "connected" : "disconnected");
}

void handle_appfocus(bool in_focus){
    if (in_focus) {
        handle_bluetooth(bluetooth_connection_service_peek());
        //handle_battery(battery_state_service_peek());
    }
}

static void handle_battery(BatteryChargeState charge_state) {
    layer_mark_dirty(battery_layer);

    static char battery_text[] = "100%";
    if (charge_state.is_charging) {
      snprintf(battery_text, sizeof(battery_text), "charging");
      battery_count = -1;
    }else{
      snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
      text_layer_set_text(text_battery_layer, battery_text);

      battery_count = (charge_state.charge_percent / 10 + 1) / 2;
    }
}


void handle_deinit(void) {
  gbitmap_destroy(icon_bt);
  tick_timer_service_unsubscribe();
}

static void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);

  //icon image
  icon_bt = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH);

  // text_date_layer = text_layer_create(GRect(8, 68, 144-8, 168-68));
  // text_layer_set_text_color(text_date_layer, GColorWhite);
  // text_layer_set_background_color(text_date_layer, GColorClear);
  // text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  // layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_nldate_layer = text_layer_create(GRect(8, 40, 144-8, 30));
  text_layer_set_text_color(text_nldate_layer, GColorWhite);
  text_layer_set_background_color(text_nldate_layer, GColorClear);
  text_layer_set_font(text_nldate_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IPAG_20)));
  layer_add_child(window_layer, text_layer_get_layer(text_nldate_layer));

  //ext_date_layer = text_layer_create(GRect(8, 20, 144-8, 168-68));
  ext_date_layer = text_layer_create(GRect(8, 68, 144-8, 30));
  text_layer_set_text_color(ext_date_layer, GColorWhite);
  text_layer_set_background_color(ext_date_layer, GColorClear);
  text_layer_set_font(ext_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IPAG_21)));
  layer_add_child(window_layer, text_layer_get_layer(ext_date_layer));

  text_time_layer = text_layer_create(GRect(7, 96, 144-32, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);  //RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49
  text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_42)));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  //GRect(x, y, width, height);
  text_sec_layer = text_layer_create(GRect(120, 118, 30, 20));
  text_layer_set_text_color(text_sec_layer, GColorWhite);
  text_layer_set_background_color(text_sec_layer, GColorClear);
  text_layer_set_font(text_sec_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_20)));
  layer_add_child(window_layer, text_layer_get_layer(text_sec_layer));


  GRect line_frame = GRect(8, 97, 139, 2);
  line_layer = layer_create(line_frame);
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);

  //bluetooth
  // bt_layer = layer_create(GRect(83,56,9,12)); //9*12
  // layer_set_update_proc(bt_layer, &bt_layer_update_callback);
  // layer_add_child(window_layer, bt_layer);
  btimap_bt_layer = bitmap_layer_create(GRect(4,4,9,14)); //9*12
  bitmap_layer_set_bitmap(btimap_bt_layer, icon_bt);
  layer_add_child(window_layer, bitmap_layer_get_layer(btimap_bt_layer));

  //Battery status icon
  battery_layer = layer_create(GRect(12, 5, 40, 30));
  layer_set_update_proc(battery_layer, &battery_layer_update_callback);
  layer_add_child(window_layer, battery_layer);

  text_battery_layer = text_layer_create(GRect(42,2,40,20));
  text_layer_set_text_color(text_battery_layer, GColorWhite);
  text_layer_set_background_color(text_battery_layer, GColorClear);
  //text_layer_set_font(text_battery_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IPAG_17)));
  text_layer_set_font(text_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_battery_layer));

  //sava loacl time 
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);
  //handle_sec_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);
  // TODO: Update display here to avoid blank display on launch?

  app_focus_service_subscribe(&handle_appfocus);
  // bluetooth
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  // battery 
  battery_state_service_subscribe(&handle_battery);
  handle_battery(battery_state_service_peek());
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
