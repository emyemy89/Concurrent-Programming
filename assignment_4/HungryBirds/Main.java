public class Main{



    public static void main(String[] args){

        int babyBirdCount;
        int wormCount;
        if(args.length>0){
            babyBirdCount=Integer.parseInt(args[0]);
        }else{
            babyBirdCount=BabyBirds.BABIES_COUNT;
        }

        if(args.length>1){
            wormCount=Integer.parseInt(args[1]);
        }else{
            wormCount=Dish.WORM_COUNT;
        }

        // create instances
        Dish dish= new Dish(wormCount);
        ParentBird parentBird= new ParentBird(dish);


        //create threads
        Thread parentTh = new Thread(parentBird);
        parentTh.start();

        Thread[] babyThreads = new Thread[babyBirdCount];
        for (int i = 0; i < babyBirdCount; i++) {
            BabyBirds babyBirds = new BabyBirds(dish, i);
            babyThreads[i] = new Thread(babyBirds);
            babyThreads[i].start();
        }


        //join
        try {
            parentTh.join();
            for (int i = 0; i < babyBirdCount; i++) {
                babyThreads[i].join();
            }
        }catch(InterruptedException e){ }

    }
}