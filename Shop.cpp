/*
Base code provided by rtdimpsey.
Modifications by Nayana Yeshlur for CSS 503 Program 2.

This program implements the sleeping barbers problem with multiple barber and customer threads.

*/
#include "Shop.h"
using namespace std;

//A function used by the Shop constructors to initialize data members.
void Shop::init() 
{

   customer_in_chair_ = new int[barber];
   in_service_ = new bool[barber];
   money_paid_ = new bool[barber];

   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);

   cond_customer_served_ = new pthread_cond_t[barber];
   cond_barber_paid_ = new pthread_cond_t[barber];
   cond_barber_sleeping_ = new pthread_cond_t[barber];

   for (int i = 0; i < barber; i++)
   {
      customer_in_chair_[i] = 0;
      in_service_[i] = false;
      money_paid_[i] = false;

      pthread_cond_init(&cond_customer_served_[i], NULL);
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
   }
}

//Takes in int, returns string representation
string Shop::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

//outputs customer[id] or barber[id] depending on sign of person int, and outputs corresponding message string
void Shop::print(int person, string message)
{
   if(person > 0) {
      cout << "customer[" << person << "]: " << message << endl;
   } else {
      person = person * -1;
      cout << "barber[" << person << "]: " << message << endl;
   }
}

//method to get number of customers dropped due to no available seats
int Shop::get_cust_drops() const
{
    return cust_drops_;
}

//Method that represents customer (with id customerID) visiting the barber shop
//They will either leave because there are no chairs, wait in a waiting chair, or start a hair cut with a barber
//If the customer does not get a hair cut the method will return -1, if the customer does get a haircut
//the method returns the id of the barber who services that customer.
int Shop::visitShop(int customerID) 
{
   pthread_mutex_lock(&mutex_);
   
   //if there are no waiting chairs but there are service/barber chairs open
   if(waiting_chairs_.size() == 0 && hasChair() != -1) {
      int availableChair = hasChair();
      if (customer_in_chair_[availableChair] == 0) {
         customer_in_chair_[availableChair] = customerID;             // have the service chair
         in_service_[availableChair] = true;
      }

      print(customerID, "moves to service chair [" + int2string(availableChair) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));

      //signal for barber to wake up 
      pthread_cond_signal(&cond_barber_sleeping_[availableChair]);

      pthread_mutex_unlock(&mutex_); 
      return availableChair;
   }

   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_)
   {
      print(customerID,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1; //return false;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   //if there are no barber/service chairs available but there are waiting chairs available, take waiting chair 
   if(hasChair() == -1 || !waiting_chairs_.empty())
   {
      //while customer is waiting, push onto wait queue
      waiting_chairs_.push(customerID);
      print(customerID, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      //If there are no barber chairs available, thread should wait on cond_customers_waiting condition
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);

      //once thread can continue it should be popped from the waiting queue
      waiting_chairs_.pop();
   }
   
   //find available chair, put customer in that chair, indicate that chair is in service
   int availableChair = hasChair();
   if (customer_in_chair_[availableChair] == 0) {
      customer_in_chair_[availableChair] = customerID;             // have the service chair
      in_service_[availableChair] = true;
    }

   print(customerID, "moves to a service chair[" + int2string(availableChair) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_[availableChair]);

   pthread_mutex_unlock(&mutex_); 
   return availableChair; //return true;
}

//This method represents a customer (with ID customerID) getting their hair cut and then paying and saying goodbye
//to the barber with id barberID
void Shop::leaveShop(int customerID, int barberID) 
{
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(customerID, "wait for barber[" + int2string(barberID) + "] to be done with hair-cut");
   while (in_service_[barberID] == true)
   {
      //thread waiting on condition of customer being served
      //specifically customer in barber's chair who has barberID
      pthread_cond_wait(&cond_customer_served_[barberID], &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   money_paid_[barberID] = true;

   //signals thread that barber has been paid
   pthread_cond_signal(&cond_barber_paid_[barberID]);
   print(customerID, "says good-bye to barber [" + int2string(barberID) + "]");
   pthread_mutex_unlock(&mutex_);
}

//method to coordinate what barber is doing
//if there is no customer in the barber's chair or the waiting chairs are empty, then the barber sleeps
//if there is a customer in the barber's chair they start a hair cut
void Shop::helloCustomer(int barberID) 
{
   pthread_mutex_lock(&mutex_);
   
   // If no customers than barber can sleep
   if ((waiting_chairs_.empty()) && (customer_in_chair_[barberID] == 0)) 
   {
      print(((barberID) * -1), "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_[barberID], &mutex_);
   }

   if (customer_in_chair_[barberID] == 0)               // check if the customer, sit down.
   {
       pthread_cond_wait(&cond_barber_sleeping_[barberID], &mutex_);
   }

   print(((barberID) * -1), "starts a hair-cut service for customer[" + int2string( customer_in_chair_[barberID]) + "]");
   pthread_mutex_unlock(&mutex_);
}

//In this method customer's service is completed, and barber call in next customer
void Shop::byeCustomer(int barberID) 
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_[barberID] = false;
  print(((barberID) * -1), "says he's done with a hair-cut service for customer[" + int2string(customer_in_chair_[barberID]) + "]");
  money_paid_[barberID] = false;

  //thread signaled that customer has been served (customer sitting in barber's (barberID's) chair)
  pthread_cond_signal(&cond_customer_served_[barberID]);
  while (money_paid_[barberID] == false)
  {
     //waiting on barber to be paid
      pthread_cond_wait(&cond_barber_paid_[barberID], &mutex_);
  }

  //Signal to customer to get next one
  customer_in_chair_[barberID] = 0;
  print(((barberID) * -1), "calls in another customer");

  //signal for next waiting customer to start service
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}

//checks to see where there is an empty chair, returns -1 if there are no empty chairs
int Shop::hasChair(){
    for(int i=0; i< barber; i++){
        if(customer_in_chair_[i]==0){
            return i;
        }
    }
    return -1;
}
