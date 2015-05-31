#include "window.h"
#include <pebble.h>

float myModFunc(float x, float y) {
	return x - y*(int)(x/y);
}
	
static GBitmap *map_full;
static GBitmap *map_part;

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_droid_serif_28_bold;
static BitmapLayer *s_map_display;
static TextLayer *s_time_display;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  
  s_res_droid_serif_28_bold = fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD);
  // s_map_display
  s_map_display = bitmap_layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_map_display);
  
  // s_time_display
  #ifdef PBL_COLOR
    s_time_display = text_layer_create(GRect(0, 57, 144, 30));
  #else
    s_time_display = text_layer_create(GRect(0, 127, 144, 30));
  #endif
  text_layer_set_background_color(s_time_display, GColorClear);
  text_layer_set_text_color(s_time_display, GColorWhite);
  text_layer_set_text(s_time_display, "00:00");
  text_layer_set_text_alignment(s_time_display, GTextAlignmentCenter);
  text_layer_set_font(s_time_display, s_res_droid_serif_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_time_display);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  bitmap_layer_destroy(s_map_display);
  text_layer_destroy(s_time_display);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_window(void) {
  initialise_ui();
	
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_window(void) {
  window_stack_remove(s_window, true);
}

void handle_minute_tick(struct tm * tick_time, TimeUnits units_changed) {
	static char time_text[] = "00:00";;
	clock_copy_time_string(time_text, sizeof(time_text));
	text_layer_set_text(s_time_display, time_text);
	
	time_t local_time = time(NULL);
	struct tm * utc_time = gmtime(&local_time);
	app_log(APP_LOG_LEVEL_DEBUG, "window.c", 67, "UTC time: %i:%i", utc_time->tm_hour, utc_time->tm_min);
	float hours_for_placement = utc_time->tm_hour + (utc_time->tm_min/60.0); // Uses minutes as well as hours for more precision

	hours_for_placement = myModFunc(hours_for_placement, 24);
	int img_x_placement =  432 - (18 * hours_for_placement);
	
	map_part = gbitmap_create_as_sub_bitmap(map_full, GRect(img_x_placement,0,144,168));
	bitmap_layer_set_bitmap(s_map_display, map_part);
	
	app_log(APP_LOG_LEVEL_DEBUG, "window.c", 80, "h (map placement formula): %f", hours_for_placement);
	app_log(APP_LOG_LEVEL_DEBUG, "window.c", 81, "local time: %i:%i", tick_time->tm_hour, tick_time->tm_min);
	app_log(APP_LOG_LEVEL_DEBUG, "window.c", 82, "Map display: %ipx - %ipx", img_x_placement, img_x_placement+144);
}

void handle_init()
{
	show_window();
	
	map_full = gbitmap_create_with_resource(RESOURCE_ID_MAP_IMAGE);
	map_part = gbitmap_create_as_sub_bitmap(map_full, GRect(0,0,144,168));
	bitmap_layer_set_bitmap(s_map_display, map_part);
	
	time_t* theCurrentTime = malloc(sizeof(*theCurrentTime));
	time(theCurrentTime);
	handle_minute_tick(localtime(theCurrentTime), MINUTE_UNIT);
	
	tick_timer_service_subscribe(MINUTE_UNIT,handle_minute_tick);	
}
void handle_deinit()
{
	//destroy_ui();
	tick_timer_service_unsubscribe();
}
int main(void)
{
	handle_init();
	app_event_loop();
	handle_deinit();
}