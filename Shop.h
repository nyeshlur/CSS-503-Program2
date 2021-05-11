/*
Base code provided by rtdimpsey.
Modifications by Nayana Yeshlur for CSS 503 Program 2.
*/
#ifndef SHOP_H__ 
#define SHOP_H_ 

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 1
#define kDefaultNumBarbers 1

class Shop 
{
public:
   Shop(int num_barbers, int num_chairs) : max_waiting_cust_((num_chairs >= 0 ) ? num_chairs : kDefaultNumChairs), 
   cust_drops_(0), barber((num_barbers > 0) ? num_barbers : kDefaultNumBarbers)
   { 
      init(); 
   };
   Shop() : max_waiting_cust_(kDefaultNumChairs), barber(kDefaultNumBarbers), cust_drops_(0)
   { 
      init(); 
   };

   int visitShop(int customerID);   // return barber id only when a customer got a service
   void leaveShop(int customerID, int barberID);
   void helloCustomer(int barberID);
   void byeCustomer(int barberID);
   int get_cust_drops() const; //does this need to be const?

 private:
   const int max_waiting_cust_;              // the max number of threads that can wait
   const int barber; // the id of the barber thread, was static const int
   int *customer_in_chair_;
   bool *in_service_;            
   bool *money_paid_;
   queue<int> waiting_chairs_;  // includes the ids of all waiting threads
   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   pthread_cond_t  *cond_customer_served_;
   pthread_cond_t  *cond_barber_paid_;
   pthread_cond_t  *cond_barber_sleeping_;
  
   void init();
   string int2string(int i);
   void print(int person, string message);
   int hasChair();
};
#endif
