import java.util.Random;

public class SpaceSim {

  public static void main(String[] args) {
    int V = 3; //number of concurrent vehicles
    int N = 500; //max nitrogen
    int Q = 500; //max quantum fluid

    Station station = new Station(V, N, Q);

    int number_of_vehicles = 5;
    int number_of_suppliers = 3;

    SpaceV[] vehicles = new SpaceV[number_of_vehicles];
    SupplyV[] suppliers = new SupplyV[number_of_suppliers];

    for (int i = 0; i < number_of_vehicles; i++) {
        vehicles[i] = new SpaceV(station, 50, 50);
        vehicles[i].start();
    }

    for (int i = 0; i < number_of_suppliers; i++) {
        suppliers[i] = new SupplyV(station, 350,400 );
        suppliers[i].start();
    }

    for (int i = 0; i < number_of_vehicles; i++) {
        try {
          vehicles[i].join();
        } catch (InterruptedException e) {}
      }
  
      //Join supplier threads
      for (int i = 0; i < number_of_suppliers; i++) {
        try {
          suppliers[i].join();
        } catch (InterruptedException e) {}
      }

      System.out.println("Program finished");
  }

  //Thread that refuels from station
  static class SpaceV extends Thread {

    private Station station;
    int N_requested, Q_requested;
    int nb_of_visits;

    private Random random;

    public SpaceV(Station station, int N_requested, int Q_requested) {
      this.station = station;
      this.N_requested = N_requested;
      this.Q_requested = Q_requested;
      this.random = new Random();
    }

    @Override
    public void run() {
      while (nb_of_visits < 5) {
        station.request_fuel(N_requested, Q_requested); //get fuel
        nb_of_visits++;
        try { //sleep
          Thread.sleep(random.nextInt(2000)); //to simulate spacce travel
        } catch (InterruptedException e) {}
        
      }
      System.out.println("Vehicle finished refueling");
    }
  }

  //Thread that brings fuel to the station
  static class SupplyV extends Thread {

    private Station station;
    int N_supplied, Q_supplied;
    private Random random;

    public SupplyV(Station station, int N_supplied, int Q_supplied) {
      this.station = station;
      this.N_supplied = N_supplied;
      this.Q_supplied = Q_supplied;
      this.random = new Random();
    }

    @Override
    public void run() {
      station.refuel_station(N_supplied, Q_supplied); //supply fuel

      station.request_fuel(random.nextInt(50), random.nextInt(50)); //get fuel

      try { //sleep
        Thread.sleep(random.nextInt(4000));//to simulate travel
      } catch (InterruptedException e) {}
    }
  }
}
