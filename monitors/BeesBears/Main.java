public class Main{



    public static void main(String[] args){

        int beeCount;
        int capacity;
        if(args.length>0){
            beeCount=Integer.parseInt(args[0]);
        }else{
            beeCount=Bees.BEES_COUNT;
        }

        if(args.length>1){
            capacity=Integer.parseInt(args[1]);
        }else{
            capacity=HoneyPot.CAPACITY;
        }

        // create instances
        HoneyPot honeyPot= new HoneyPot(capacity);
        Bear bear= new Bear(honeyPot);


        //create threads
        Thread bearTh = new Thread(bear);
        bearTh.start();

        Thread[] beeThreads = new Thread[beeCount];
        for (int i = 0; i < beeCount; i++) {
            Bees bees = new Bees(honeyPot, i);
            beeThreads[i] = new Thread(bees);
            beeThreads[i].start();
        }


        //join
        try {
            bearTh.join();
            for (int i = 0; i < beeCount; i++) {
                beeThreads[i].join();
            }
        }catch(InterruptedException e){ }

    }
}