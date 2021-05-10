/*
Base code provided by rtdimpsey.
*/
#include "Shop.h"
using namespace std;

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

   //int default_customer_in_chair_ = 0;
   //bool default_in_service_ = false;
   //bool default_money_paid_ = false;

   for (int i = 0; i < barber; i++)
   {
      customer_in_chair_[i] = 0; //default_customer_in_chair_;
      in_service_[i] = false; //default_in_service_;
      money_paid_[i] = false; //default_money_paid_;

      pthread_cond_init(&cond_customer_served_[i], NULL);
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
   }
}

string Shop::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

void Shop::print(int person, string message)
{
   if(person > 0) {
      cout << "customer[" << person << "]: " << message << endl;
   } else {
      person = person * -1;
      cout << "barber[" << person << "]: " << message << endl;
   }
}

int Shop::get_cust_drops() const
{
    return cust_drops_;
}

int Shop::visitShop(int customerID) 
{
   pthread_mutex_lock(&mutex_);
   
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
   //if (customer_in_chair_ != 0 || !waiting_chairs_.empty()) 
   if(hasChair() == -1 || !waiting_chairs_.empty())
   {
      waiting_chairs_.push(customerID);
      print(customerID, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      waiting_chairs_.pop();
   }
   bool test = waiting_chairs_.empty();

   //cout << "test" << test << endl;

   
   int availableChair = hasChair();
   if (customer_in_chair_[availableChair] == 0) {
      customer_in_chair_[availableChair] = customerID;             // have the service chair
      in_service_[availableChair] = true;
    }

   print(customerID, "moves to a service chair[" + int2string(availableChair) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   
   //customer_in_chair_ = id;
   //in_service_ = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_[availableChair]);

   pthread_mutex_unlock(&mutex_); 
   return availableChair; //return true;
}

void Shop::leaveShop(int customerID, int barberID) 
{
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(customerID, "wait for barber[" + int2string(barberID) + "] to be done with hair-cut");
   while (in_service_[barberID] == true)
   {
      pthread_cond_wait(&cond_customer_served_[barberID], &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   money_paid_[barberID] = true;
   pthread_cond_signal(&cond_barber_paid_[barberID]);
   print(customerID, "says good-bye to barber [" + int2string(barberID) + "]");
   pthread_mutex_unlock(&mutex_);
}

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

void Shop::byeCustomer(int barberID) 
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_[barberID] = false;
  print(((barberID) * -1), "says he's done with a hair-cut service for customer[" + int2string(customer_in_chair_[barberID]) + "]");
  money_paid_[barberID] = false;
  pthread_cond_signal(&cond_customer_served_[barberID]);
  while (money_paid_[barberID] == false)
  {
      pthread_cond_wait(&cond_barber_paid_[barberID], &mutex_);
  }

  //Signal to customer to get next one
  customer_in_chair_[barberID] = 0;
  print(((barberID) * -1), "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}

int Shop::hasChair(){
    for(int i=0; i< barber; i++){
        if(customer_in_chair_[i]==0){
            return i;
        }
    }
    return -1;
}
