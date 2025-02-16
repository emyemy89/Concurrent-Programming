

public class ParentBird implements Runnable{

    //create dish
    Dish dish;

    public ParentBird(Dish dish){
        this.dish=dish;
    }

    public void run(){
        //check continously
        while(true){
            try {
                dish.putWorm();
                Thread.sleep(1000);
            }catch(InterruptedException e){ }
        }
    }

}