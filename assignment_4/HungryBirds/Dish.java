import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;


public class Dish {


    int wormCount;
    boolean empty=false;

    public static final int WORM_COUNT = 10;

    // constructor
    public Dish(int wormCount) {
        this.wormCount = wormCount;
    }



    synchronized void getWorm(int birdId) throws InterruptedException {
        while(empty){
            try {
                System.out.println("Waiting to get more food");
                wait();
            }catch(InterruptedException e){}
        }
        wormCount--;
        System.out.println("Baby bird "+birdId +" is eating a worm. Worms left: " + wormCount);
        if(wormCount==0){
            empty=true;
            notifyAll();
        }
    }

    synchronized void putWorm()throws InterruptedException {
        while(!empty){
            try{
                wait();
            }catch(InterruptedException e){ }
        }
        this.wormCount=WORM_COUNT;
        System.out.println("Refilling the dish");
        empty=false;
        notifyAll();
    }

}