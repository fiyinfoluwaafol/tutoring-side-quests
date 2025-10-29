#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "BENSCHILLIBOWL.h"

// Feel free to play with these numbers!
#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread function that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = *((int*) tid);
    printf("Customer #%d is entering the restaurant.\n", customer_id);

    for (int n = 0; n < ORDERS_PER_CUSTOMER; n++) {
        Order *new_order = (Order*) malloc(sizeof(Order));  // Allocate memory for an order

        MenuItem menu_item = PickRandomMenuItem();  // Pick a random menu item

        new_order->customer_id = customer_id;  // Populate the order
        new_order->menu_item = menu_item;
        new_order->next = NULL;

        int order_number = AddOrder(bcb, new_order);  // Add the order to the restaurant
        printf("Customer #%d added Order #%d: %s\n", customer_id, order_number, menu_item);
    }

    printf("Customer #%d has placed all orders.\n", customer_id);
    return NULL;
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then free the space taken by the order.
 * The cook should take orders from the restaurants until it does not receive any orders.
 */
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = *((int*) tid);
    int orders_fulfilled = 0;

    printf("Cook #%d is ready to work.\n", cook_id);

    Order *order_to_cook = GetOrder(bcb);

    while (order_to_cook) {
        printf("Cook #%d is fulfilling Order #%d for Customer #%d: %s\n",
               cook_id, order_to_cook->order_number, order_to_cook->customer_id, order_to_cook->menu_item);

        free(order_to_cook);  // Free the memory of the fulfilled order
        orders_fulfilled++;
        order_to_cook = GetOrder(bcb);
    }

    printf("Cook #%d has fulfilled %d orders and is done for the day.\n", cook_id, orders_fulfilled);
    return NULL;
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant.
 *  - create customers and cooks.
 *  - wait for all customers and cooks to be done.
 *  - close the restaurant.
 */
int main() {
    srand(time(NULL));  // Seed the random number generator

    // Open the restaurant
    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);

    pthread_t customers[NUM_CUSTOMERS];
    pthread_t cooks[NUM_COOKS];

    int customer_id[NUM_CUSTOMERS];
    int cook_id[NUM_COOKS];

    // Create customer threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        customer_id[i] = i + 1;
        pthread_create(&customers[i], NULL, BENSCHILLIBOWLCustomer, &customer_id[i]);
    }

    // Create cook threads
    for (int i = 0; i < NUM_COOKS; i++) {
        cook_id[i] = i + 1;
        pthread_create(&cooks[i], NULL, BENSCHILLIBOWLCook, &cook_id[i]);
    }

    // Wait for all customers to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    // Wait for all cooks to finish
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_join(cooks[i], NULL);
    }

    // Close the restaurant
    CloseRestaurant(bcb);

    return 0;
}