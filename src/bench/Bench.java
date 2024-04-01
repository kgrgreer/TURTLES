public class Bench {

  public static void main (String[] args)
    throws java.lang.Exception
  {
    long count = 0;

    long max = Long.parseLong(args[0]);

    long start = System.currentTimeMillis();
    for ( long i = 0 ; i < max ; i++ ) count++;
    long end = System.currentTimeMillis();

    System.out.println(count + " " + (end-start) + " " + max);
  }
}
