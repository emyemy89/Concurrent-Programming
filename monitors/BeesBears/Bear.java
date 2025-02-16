public class Bear implements Runnable{



    HoneyPot honeyPot;

    public Bear(HoneyPot honeyPot){
        this.honeyPot=honeyPot;
    }

    public void run(){
        while(true){
            try{
                honeyPot.getHoney();
                Thread.sleep(100);
            }catch(InterruptedException e){ }
        }
    }

}