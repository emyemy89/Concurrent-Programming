public class Bees implements Runnable{


    public static final int BEES_COUNT = 5;
    int beeId;


    //create dish
    HoneyPot honeyPot;

    public Bees(HoneyPot honeyPot, int beeId){
        this.honeyPot=honeyPot;
        this.beeId=beeId;
    }

    public void run(){
        //check continously
        while(true){
            try {
                honeyPot.putHoney(beeId);
                Thread.sleep(1000);
            }catch(InterruptedException e){ }
        }
    }

}