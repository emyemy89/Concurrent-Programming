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

  public void request_fuel(int requested_N, int requesed_Q) {
    synchronized (this) {
      while (V_available == 0) { //wait for open slot
        try {
          wait();
        } catch (InterruptedException e) {}
      }

      V_available--;

      while (requested_N > N_available || requesed_Q > Q_available) { //wait for fuel without blocking
        try {
          wait();
        } catch (InterruptedException e) {}
      }

      N_available -= requested_N;
      Q_available -= requesed_Q;
      System.out.println(
        Thread.currentThread().getName() +
        " has recieved " +
        requested_N +
        " Nitrogen and " +
        requesed_Q +
        " quantum fuel"
      );
    }
    try {
      Thread.sleep(500);
    } catch (InterruptedException e) {}

    leave_station();
  }

  public synchronized void leave_station() {
    V_available++;
    notifyAll();
    System.out.println(
      Thread.currentThread().getName() + " has left the station"
    );
  }

  public synchronized void refuel_station(int N_supplied, int Q_supplied) {
    while (N_available + N_supplied > N && Q_available + Q_supplied > Q) {
      try {
        wait();
      } catch (InterruptedException e) {}
    }

    N_available += N_supplied;
    Q_available += Q_supplied;
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
