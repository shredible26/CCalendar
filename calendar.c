#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"
#include "calendar.h"

/* initializes a Calendar structure based on the parameters provided */
int init_calendar(const char *name, int days,
		  int (*comp_func) (const void *ptr1, const void *ptr2),
		  void (*free_info_func) (void *ptr), Calendar ** calendar) {

    if ((*calendar) && name && days >= 1) {
        (*calendar) = malloc(sizeof(Calendar));

        if (calendar) {
            (*calendar)->name = malloc(sizeof(char) * (strlen(name) + 1));
            strcpy((*calendar)->name, name);
            (*calendar)->events = calloc(days, sizeof(Event));
            (*calendar)->days = days;
            (*calendar)->total_events = 0;
            (*calendar)->comp_func = comp_func;
            (*calendar)->free_info_func = free_info_func;
            return SUCCESS;
        }
    }
    return FAILURE;
}

int print_calendar(Calendar *calendar, FILE *output_stream, int print_all) {
    Event *event_ptr;
    int x = 0;

    if (calendar && output_stream) {
        if (print_all) {
            fprintf(output_stream, "Calendar's Name: \"%s\"\n", calendar->name);
            fprintf(output_stream, "Days: %d\n", calendar->days);
            fprintf(output_stream, "Total Events: %d\n\n", calendar->total_events);
        }
        fprintf(output_stream, "**** Events ****\n");
        if (calendar->total_events > 0) {
            for (x = 0; x < calendar->days; x++) {
                printf("Day %d\n", x + 1);
                event_ptr = calendar->events[x];
                while (event_ptr) {
                    fprintf(output_stream, "Event's Name: \"%s\", ", event_ptr->name);
                    fprintf(output_stream, "Start_time: %d, ", event_ptr->start_time);
                    fprintf(output_stream, "Duration: %d\n", event_ptr->duration_minutes);
                    event_ptr = event_ptr->next;
                }
            }
        }
        return SUCCESS;
    }
    return FAILURE;
}

int add_event(Calendar *calendar, const char *name, int start_time,
	      int duration_minutes, void *info, int day) {
    Event *new_event, *prev_ptr, *event_ptr;
    Event *event; /* temp event to use for find_event */

    if (calendar && name && (start_time >= 0 && start_time <= 2400)) {
        if (duration_minutes > 0 && day > 0 && day <= calendar->days) {
            if (find_event(calendar, name, &event) == FAILURE) {
                new_event = malloc(sizeof(Event));
                new_event->name = malloc(sizeof(char) * (strlen(name) + 1));
                strcpy(new_event->name, name);
                new_event->duration_minutes = duration_minutes;
                new_event->start_time = start_time;
                new_event->info = info;
                event_ptr = calendar->events[day - 1];
                if (event_ptr == NULL) {
                    calendar->events[day - 1] = new_event;
                    new_event->next = NULL;
                }
                else if (event_ptr != NULL) {
                    if (calendar->comp_func(event_ptr, new_event) > 0) {
                        calendar->events[day - 1] = new_event;
                        new_event->next = event_ptr;
                    }
                    else {
                        prev_ptr = event_ptr;
                        event_ptr = event_ptr->next;
                        while (event_ptr && calendar->comp_func(event_ptr, new_event) <= 0) {
                            prev_ptr = event_ptr;
                            event_ptr = event_ptr->next;
                        }
                        new_event->next = event_ptr;
                        prev_ptr->next = new_event;
                    }
                } 
                (calendar->total_events)++;
                return SUCCESS;
            }
        }
    }
    return FAILURE;
}

int find_event(Calendar *calendar, const char *name, Event **event) {
    Event *event_ptr;
    int x = 0;

    if (calendar && name) {
        for (x = 0; x < calendar->days; x++) {
            event_ptr = calendar->events[x];
            while (event_ptr && strcmp(event_ptr->name, name) != 0) {
                event_ptr = event_ptr->next;
            }
            if (event_ptr && event) {
                *event = event_ptr;
                return SUCCESS;
            }
        }
    }
    return FAILURE;
}

int find_event_in_day(Calendar *calendar, const char *name, int day,
		      Event **event) {
    Event *event_ptr;
    int x = 0;

    if (calendar && name && day > 0 && day <= calendar->days) {

        for (x = 0; x < calendar->days; x++) {
            if ((x + 1) == day) {

                event_ptr = calendar->events[x];
                while (event_ptr && strcmp(event_ptr->name, name) != 0) {
                    event_ptr = event_ptr->next;
                }
                if (event_ptr && event) {
                    *event = event_ptr;
                    return SUCCESS;
                }
            }
        } 
    }
    return FAILURE;
}
int remove_event(Calendar *calendar, const char *name) {
    Event **event = NULL, *prev_ptr, *event_ptr;
    int x = 0;

    if (calendar && name && find_event(calendar, name, event) == FAILURE) {
        for (x = 0; x < calendar->days; x++) {
            event_ptr = calendar->events[x];
            if (event_ptr != NULL) {
                if (strcmp(event_ptr->name, name) == 0) {
                    calendar->events[x] = event_ptr->next;
                    event_ptr->next = NULL;
                }
                else {
                    while (event_ptr && strcmp(event_ptr->name, name) != 0) {
                        prev_ptr = event_ptr;
                        event_ptr = event_ptr->next;
                    }
                    prev_ptr->next = event_ptr->next;
                    event_ptr->next = NULL;
                }
                free(event_ptr->name);
                if (calendar->free_info_func) {
                    calendar->free_info_func(event_ptr->info);
                }
                free(event_ptr);
                (calendar->total_events)--;
                return SUCCESS;
            }
        } 
    }
    return FAILURE;
}

void *get_event_info(Calendar *calendar, const char *name) {
   Event *event;

   if (find_event(calendar, name, &event) == FAILURE) {
      return NULL;
   }
   return event->info;
}

int clear_calendar(Calendar *calendar) {
    int x = 0;

    if (calendar) {
        for (x = 0; x < calendar->days; x++) {
            clear_day(calendar, (x + 1));
        }
        return SUCCESS;
    }
    return FAILURE;
}

int clear_day(Calendar *calendar, int day) {
    Event *temp, *event_ptr;

    if (calendar && day > 0 && day <= calendar->days) {
        if (calendar->total_events > 0) {
            event_ptr = calendar->events[day - 1];
            while (event_ptr) {
                temp = event_ptr;
                event_ptr = event_ptr->next;
                free(temp->name);
                if (calendar->free_info_func && temp->info) {
                    calendar->free_info_func(temp->info);
                }
                free(temp);
                (calendar->total_events)--;
            }
	    calendar->events[day - 1] = NULL;
            return SUCCESS;
        }
    }
    return FAILURE;
}

int destroy_calendar(Calendar *calendar) {
    if (calendar) {
        free(calendar->name);
        clear_calendar(calendar);
        free(calendar->events);
        free(calendar);
        return SUCCESS;
    }
    return FAILURE;
}
