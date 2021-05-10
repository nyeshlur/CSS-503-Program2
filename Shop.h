/*
Base code provided by rtdimpsey.
*/
#ifndef SHOP_H__ 
#define SHOP_H_ 

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultNumBarbers 1

class Shop 
{
public:
   Shop(int num_barbers, int num_chairs) : max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), customer_in_chair_(0),
      in_service_(false), money_paid_(false), cust_drops_(0), max_num_barbers_((num_barbers > 0) ? num_barbers : kDefaultNumBarbers)
   { 
      init(); 
   };
   Shop() : max_waiting_cust_(kDefaultNumChairs), max_num_barbers_(kDefaultNumBarbers), customer_in_chair_(0), 
      in_service_(false), money_paid_(false), cust_drops_(0)
   { 
      init(); 
   };

   int visitShop(int id);   // return barber id only when a customer got a service
   void leaveShop(int id, int barber);
   void helloCustomer(int id);
   void byeCustomer(int id);
   int get_cust_drops() const;

 private:
   const int max_waiting_cust_;              // the max number of threads that can wait
   const int max_num_barbers_;
   int customer_in_chair_;
   bool in_service_;            
   bool money_paid_;
   queue<int> waiting_chairs_;  // includes the ids of all waiting threads
   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   pthread_cond_t  cond_customer_served_;
   pthread_cond_t  cond_barber_paid_;
   pthread_cond_t  cond_barber_sleeping_;

   static const int barber = 0; // the id of the barber thread
  
   void init();
   string int2string(int i);
   void print(int person, string message);
};
#endif
