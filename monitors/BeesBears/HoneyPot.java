public class HoneyPot {


    int capacity;
    int currentHoney = 0;
    boolean fullPot=false;

    public static final int CAPACITY = 10;

    // constructor
    public HoneyPot(int capacity) {
        this.capacity = capacity;
    }



    //the bear eats the honey
    synchronized void getHoney() throws InterruptedException {
        while(!fullPot){
            try {
                System.out.println("Waiting to get more honey");
                wait();
            }catch(InterruptedException e){}
        }

        //this.capacity=0;
        currentHoney = 0;
        System.out.println("Bear is eating the honey");
        fullPot=false;
        notifyAll();

    }

    synchronized void putHoney(int beeId)throws InterruptedException {
        while(fullPot){
            try{
                wait();
            }catch(InterruptedException e){ }
        }
            System.out.println("Bee number " +beeId+ " is putting one portion of honey in the pot");
            //capacity++;
            currentHoney++;

        if(currentHoney==capacity){
            System.out.println("Bee number " + beeId+ " has filled the pot and waking up the bear");
            fullPot=true;
            notifyAll();
        }

    }

}