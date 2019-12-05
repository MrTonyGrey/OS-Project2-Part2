#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
  char menuItem[] = BENSCHILLIBOWLMenu[rand() % BENSCHILLIBOWLMenuLength];
  return menuItem;
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL *restaurant = malloc(sizeof(BENSCHILLIBOWL));
    restaurant->current_size = 0;
    restaurant->max_size = max_size;
    restaurant->orders = NULL;
    restaurant->orders_handled = 0;
    restaurant->next_order_number = 1;
    restaurant->expected_num_orders = expected_num_orders;
  
    pthread_mutex_init(&(restaurant->mutex), NULL);
    pthread_cond_init(&(restaurant->can_add_orders), NULL);
    pthread_cond_init(&(restaurant->can_get_orders), NULL);
    printf("Restaurant is open!\n");
  
    return restaurant;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    printf("No. Orders Expected: %d\n", bcb->expected_num_orders);
    printf("No. Orders. Handled: %d\n", bcb->orders_handled);
    if(bcb->expected_num_orders == bcb->orders_handled){
       pthread_mutex_destroy(&bcb->mutex);
       pthread_cond_destroy(&bcb->can_add_orders);
       pthread_cond_destroy(&bcb->can_get_orders);
    }
    free(bcb);
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&(bcb->mutex));
    while (bcb->current_size == bcb->max_size) {
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }
    bcb->current_size++;
    order->order_number = bcb->next_order_number++;
    AddOrderToBack(&(bcb->orders), order);
    
    pthread_cond_signal(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));
  
    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
    Order *order = bcb->orders;
    pthread_mutex_lock(&(bcb->mutex));
    while (bcb->current_size == 0) {
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }
    bcb->orders = bcb->orders->next;
    bcb->current_size--;
    bcb->orders_handled++;
    pthread_cond_signal(&(bcb->can_add_orders));
    if (bcb->orders_handled == bcb->expected_num_orders) {
        pthread_cond_broadcast(&(bcb->can_add_orders));
    }
    pthread_mutex_unlock(&(bcb->mutex));
    return order;
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (!orders[0]) {
        orders[0] = order;
        order->next = NULL;
    }
    Order *temp = orders[0]; 
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = order;
    order->next = NULL;
    return;
}

