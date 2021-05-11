/*
Base code provided by rtdimpsey.
Modifications by Nayana Yeshlur for CSS 503 Program 2.

This program implements the sleeping barbers problem with multiple barber and customer threads.

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
   
   //Shop constructor that takes in number of waiting chairs and number of barbers
   Shop(int num_barbers, int num_chairs) : max_waiting_cust_((num_chairs >= 0 ) ? num_chairs : kDefaultNumChairs), 
   cust_drops_(0), barber((num_barbers > 0) ? num_barbers : kDefaultNumBarbers)
   { 
      init(); 
   };

   //default constructor that uses default number of chairs and barbers
   Shop() : max_waiting_cust_(kDefaultNumChairs), barber(kDefaultNumBarbers), cust_drops_(0)
   { 
      init(); 
   };

   //Method that represents customer (with id customerID) visiting the barber shop
   //They will either leave because there are no chairs, wait in a waiting chair, or start a hair cut with a barber
   //If the customer does not get a hair cut the method will return -1, if the customer does get a haircut
   //the method returns the id of the barber who services that customer.
   int visitShop(int customerID);   // return barber id only when a customer got a service

   //This method represents a customer (with ID customerID) getting their hair cut and then paying and saying goodbye
   //to the barber with id barberID
   void leaveShop(int customerID, int barberID);

   //method to coordinate what barber is doing
   //if there is no customer in the barber's chair or the waiting chairs are empty, then the barber sleeps
   //if there is a customer in the barber's chair they start a hair cut
   void helloCustomer(int barberID);

   //In this method customer's service is completed, and barber call in next customer
   void byeCustomer(int barberID);

   //method to get number of customers dropped due to no available seats
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
  
   //A function used by the Shop constructors to initialize data members.
   void init();

   //Takes in int, returns string representation
   string int2string(int i);

   //outputs customer[id] or barber[id] depending on sign of person int, and outputs corresponding message string
   void print(int person, string message);

   //checks to see where there is an empty chair, returns -1 if there are no empty chairs
   int hasChair();
};
#endif
