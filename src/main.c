#include <pebble.h>

Window *my_window;

TextLayer *text_time_layer;

GBitmap * business_hours;
BitmapLayer * business_hours_layer;

// This function is (lovingly) ripped off from the "Simplicity" watchface
char * getTimeStr(struct tm * tick_time)
{
	// Need to be static because it's used by the system later.
	static char time_text[] = "00:00";
	
	char * time_format;
	if (clock_is_24h_style())
	{
		time_format = "%R";
	}
	else
	{
		time_format = "%I:%M";
	}
	
	strftime(time_text, sizeof(time_text), time_format, tick_time);

	// Kludge to handle lack of non-padded hour format string
	// for twelve hour clock.
	if (!clock_is_24h_style() && (time_text[0] == '0'))
	{
		memmove(time_text, &time_text[1], sizeof(time_text) - 1);
	}

	return time_text;
}


void handle_minute_tick(struct tm * tick_time, TimeUnits units_changed)
{
	text_layer_set_text(text_time_layer, getTimeStr(tick_time));
	
	float utc_offset = -7; // TODO: figure out how to access settings. https://developer.getpebble.com/2/api-reference/group___app_sync.html#ga448af36883189f6345cc7d5cd8a3cc29
	int hours_for_placement = tick_time->tm_hour + (tick_time->tm_min/60.0); // Uses minutes as well as hours, because I can.
	
	int img_x_placement = (-270 + (20*utc_offset)) + (18*hours_for_placement); // type `int` because you can't have a part of a pixel lit
	
	business_hours_layer = bitmap_layer_create((GRect) {
		.origin = { .x = img_x_placement, .y = 0 },
		.size = business_hours->bounds.size
 	});
	bitmap_layer_set_bitmap(business_hours_layer, business_hours);
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(business_hours_layer));
}

void handle_init(void)
{
	my_window = window_create();
	Layer * window_layer = window_get_root_layer(my_window);
	
	window_stack_push(my_window, true /* Animated */);
	window_set_background_color(my_window, GColorBlack);
	
	text_time_layer = text_layer_create(GRect(0, 115, 144, 168-115));//text_layer_create(GRect(0, 0, 144, 20));
	
	text_layer_set_text(text_time_layer, "00:00");
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_background_color(text_time_layer, GColorClear);
	text_layer_set_font(text_time_layer, fonts_get_system_font("RESOURCE_ID_BITHAM_42_MEDIUM_NUMBERS"));
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
	
	
	business_hours = gbitmap_create_with_resource(RESOURCE_ID_BUSINESS_HOURS_BLACK);
	
	time_t* theCurrentTime = malloc(sizeof(*theCurrentTime));
	time(theCurrentTime);
	handle_minute_tick(localtime(theCurrentTime), MINUTE_UNIT);
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit(void)
{
	window_destroy(my_window);
	text_layer_destroy(text_time_layer);
	gbitmap_destroy(business_hours);
	
	tick_timer_service_unsubscribe();
}

int main(void)
{
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
