#include "BENSCHILLIBOWL.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

// Declare helper functions before their usage to prevent warnings
bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order** orders, Order* order);

// Menu items
MenuItem BENSCHILLIBOWLMenu[] = {
    "BensChilli", "BensHalfSmoke", "BensHotDog", "BensChilliCheeseFries",
    "BensShake", "BensHotCakes", "BensCake", "BensHamburger",
    "BensVeggieBurger", "BensOnionRings"
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    int item = rand() % BENSCHILLIBOWLMenuLength;
    return BENSCHILLIBOWLMenu[item];
}

/* Allocate memory for the Restaurant and initialize mutex and condition variables */
BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL* bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->next_order_number = 1;
    bcb->orders_handled = 0;
    bcb->max_size = max_size;
    bcb->expected_num_orders = expected_num_orders;

    pthread_mutex_init(&(bcb->mutex), NULL);
    pthread_cond_init(&(bcb->can_add_orders), NULL);
    pthread_cond_init(&(bcb->can_get_orders), NULL);

    printf("Restaurant is open! (Max Size: %d, Expected Orders: %d)\n", max_size, expected_num_orders);
    return bcb;
}

/* Deallocate resources and close the restaurant */
void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    if (bcb->orders_handled != bcb->expected_num_orders) {
        fprintf(stderr, "ERROR: Not all orders were handled! (%d handled, %d expected)\n",
                bcb->orders_handled, bcb->expected_num_orders);
        exit(1);
    }

    Order* current_order = bcb->orders;
    while (current_order) {
        Order* next_order = current_order->next;
        free(current_order);
        current_order = next_order;
    }

    pthread_mutex_destroy(&(bcb->mutex));
    pthread_cond_destroy(&(bcb->can_add_orders));
    pthread_cond_destroy(&(bcb->can_get_orders));
    free(bcb);

    printf("Restaurant is closed!\n");
}

/* Add an order to the back of the queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&(bcb->mutex));

    while (bcb->current_size >= bcb->max_size) {
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }

    order->order_number = bcb->next_order_number++;
    AddOrderToBack(&(bcb->orders), order);
    bcb->current_size++;

    pthread_cond_broadcast(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));

    return order->order_number;
}

/* Get an order from the front of the queue */
Order* GetOrder(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&(bcb->mutex));

    while (bcb->current_size == 0) {
        if (bcb->orders_handled == bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }

    Order* order = bcb->orders;
    bcb->orders = order->next;
    bcb->current_size--;
    bcb->orders_handled++;

    pthread_cond_broadcast(&(bcb->can_add_orders));
    pthread_mutex_unlock(&(bcb->mutex));

    return order;
}

/* Add an order to the rear of the queue */
void AddOrderToBack(Order** orders, Order* order) {
    if (*orders == NULL) {
        *orders = order;
    } else {
        Order* current = *orders;
        while (current->next) {
            current = current->next;
        }
        current->next = order;
    }
}

/* Check if the restaurant is full */
bool IsFull(BENSCHILLIBOWL* bcb) {
    return bcb->current_size >= bcb->max_size;
}

/* Check if the restaurant is empty */
bool IsEmpty(BENSCHILLIBOWL* bcb) {
    return bcb->current_size == 0;
}