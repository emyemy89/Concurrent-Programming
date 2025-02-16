import java.util.Random;

//Monitor
public class Station {

  int V; //max nb of Vehicles that can be served
  int N; //max nb of nitrogen that can be stored
  int Q; //max nb of quantum fluid that can be stored

  int N_available, Q_available, V_available;
  Random random = new Random();

  public Station(int V, int N, int Q) {
    this.V = V;
    this.N = N;
    this.Q = Q;

    V_available = V;
    N_available = N;
    Q_available = Q;
  }

  //The vehicle will request random amounts of fuel. If there is empty space it will occupy a location. 
  //If there is enough fuel of both types it will take the fuel and sleep to simulate refueling, if not it will wait for a signal. 
  //If it waits more than timeout, it will exit the function, which means there are no supply ships anymore.
  
  public void request_fuel(int requested_N, int requested_Q) {
    synchronized (this) {
      if (requested_N == 0 && requested_Q == 0) {
        return;
      }

      while (V_available == 0) { //wait for open slot
        try {
          wait();
        } catch (InterruptedException e) {}
      }

      V_available--;

      long start_time = System.currentTimeMillis();
      long timeout = 5000; //5 seconds timeout

      while (
        (requested_N > 0 && requested_N > N_available) ||
        (requested_Q > 0 && requested_Q > Q_available)
      ) { //wait for fuel without blocking
        try {
          wait(timeout);
          if (System.currentTimeMillis() - start_time >= timeout) { //in case thread is in deadlock, exit
            leave_station();
            return;
          }
        } catch (InterruptedException e) {}
      }

      N_available -= requested_N;
      Q_available -= requested_Q;
      System.out.println(
        Thread.currentThread().getName() +
        " has recieved " +
        requested_N +
        " Nitrogen and " +
        requested_Q +
        " quantum fuel"
      );
    }
    try {
      Thread.sleep(500); //to simulate refueling time
    } catch (InterruptedException e) {}

    leave_station();
  }


  //The vehicle will leave the station and notify the other threads that there is an open slot.
  public synchronized void leave_station() {
    V_available++;
    notifyAll();
    System.out.println(
      Thread.currentThread().getName() + " has left the station"
    );
  }


//Supply vehicle will resupply the station only if there is enough space for both types of fuel and signal.
//if not it will wait for a signal. If it waits more than timeout, it will exit the function, which means there are no consuming vehicles anymore.
  public synchronized void refuel_station(int N_supplied, int Q_supplied) {
    long start_time = System.currentTimeMillis();
    long timeout = 5000; //5 seconds timeout

    while (N_available + N_supplied > N || Q_available + Q_supplied > Q) {
      try {
        wait(timeout);
        if (System.currentTimeMillis() - start_time >= timeout) { //in case thread is in deadlock, exit
          leave_station();
          return;
        }
      } catch (InterruptedException e) {}
    }

    N_available += N_supplied;
    Q_available += Q_supplied;

    try {
      Thread.sleep(500); //to simulate refueling time
    } catch (InterruptedException e) {}

    System.out.println(
      Thread.currentThread().getName() +
      " has supplied " +
      N_supplied +
      " Nitrogen and " +
      Q_supplied +
      " quantum fuel"
    );
    notifyAll();
  }
}
