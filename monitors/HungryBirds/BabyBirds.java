

public class BabyBirds implements Runnable{

    public static final int BABIES_COUNT = 5;

    Dish dish;
    int birdId;

    public BabyBirds(Dish dish,int birdId){
        this.dish=dish;
        this.birdId=birdId;
    }

    public void run(){
        while(true){
            try{
                dish.getWorm(this.birdId);
                Thread.sleep(100);
            }catch(InterruptedException e){ }
        }
    }

}