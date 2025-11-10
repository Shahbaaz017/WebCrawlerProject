import com.google.gson.Gson;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class MultiThreadedCrawler {
    private static final String STARTING_URL = "http://192.168.0.141:5000/page_0.html";
    private static final int MAX_PAGES_TO_CRAWL = 20;

    private final int maxPages;
    private final int numThreads;
    private final HttpClient httpClient;
    private final Gson gson;
    private final String baseHost;
    private final ExecutorService executor;

    private final Set<String> visitedUrls = ConcurrentHashMap.newKeySet();
    private final AtomicInteger pagesCrawled = new AtomicInteger(0);

    public MultiThreadedCrawler(String startUrl, int maxPages, int numThreads) throws URISyntaxException {
        this.maxPages = maxPages;
        this.numThreads = numThreads;
        this.httpClient = HttpClient.newBuilder().version(HttpClient.Version.HTTP_1_1).build();
        this.gson = new Gson();
        this.baseHost = new URI(startUrl).getHost();
        this.executor = Executors.newFixedThreadPool(numThreads);
        this.visitedUrls.add(startUrl);
    }

    // This method is called by a thread to process a URL and return the next one.
    private String crawlPage(String url) {
        if (pagesCrawled.get() >= maxPages) return null;

        try {
            HttpRequest request = HttpRequest.newBuilder().uri(new URI(url)).GET().build();
            HttpResponse<String> response = httpClient.send(request, HttpResponse.BodyHandlers.ofString());
            
            if (response.statusCode() == 200) {
                // Increment count only after a successful fetch
                pagesCrawled.incrementAndGet();

                Document doc = Jsoup.parse(response.body(), url);
                processPageData(doc);
                
                Element nextLink = doc.selectFirst("a:contains(Next)");
                if (nextLink != null) {
                    String absUrl = nextLink.absUrl("href");
                    if (new URI(absUrl).getHost().equals(baseHost)) {
                        return absUrl;
                    }
                }
            }
        } catch (Exception e) {
            // Ignore this page and return null
        }
        return null;
    }
    
    public void run() throws InterruptedException {
        long startTime = System.nanoTime();
        
        BlockingQueue<Future<String>> futures = new LinkedBlockingQueue<>();
        // Seed the process with the first page
        futures.add(executor.submit(() -> crawlPage(STARTING_URL)));

        while(pagesCrawled.get() < maxPages) {
            try {
                // Block and wait for the next future to complete
                Future<String> completedFuture = futures.take();
                String nextUrl = completedFuture.get();

                if (nextUrl != null && visitedUrls.add(nextUrl)) {
                    // If we found a new, unvisited URL, submit it for crawling
                    futures.add(executor.submit(() -> crawlPage(nextUrl)));
                } else if (futures.isEmpty()) {
                    // If the last future returned null and there are no more tasks, we are done.
                    break;
                }

            } catch (ExecutionException e) {
                // Ignore tasks that failed
            }
        }

        executor.shutdownNow(); // Shuts down all running and pending tasks
        
        long endTime = System.nanoTime();
        double totalTimeSec = TimeUnit.NANOSECONDS.toMillis(endTime - startTime) / 1000.0;

        Map<String, Object> results = new LinkedHashMap<>();
        results.put("language", "Java");
        results.put("version", "Multi-Threaded");
        results.put("threads", numThreads);
        results.put("pages_crawled", pagesCrawled.get());
        results.put("total_time_s", Math.round(totalTimeSec * 10000.0) / 10000.0);
        results.put("pages_per_second", Math.round((pagesCrawled.get() / totalTimeSec) * 100.0) / 100.0);

        System.out.println(gson.toJson(results));
    }

    private void processPageData(Document doc) {
        try {
            Elements rows = doc.select("table tr");
            if (rows.size() < 2) return;
            double totalClose = 0;
            for (int i = 1; i < rows.size(); i++) {
                Element row = rows.get(i);
                Elements cells = row.select("td");
                if (cells.size() > 4) {
                    totalClose += Double.parseDouble(cells.get(4).text());
                }
            }
        } catch (NumberFormatException | IndexOutOfBoundsException e) {
            // Ignore malformed rows
        }
    }

    public static void main(String[] args) throws URISyntaxException, InterruptedException {
        int numThreads = args.length > 0 ? Integer.parseInt(args[0]) : 4;
        new MultiThreadedCrawler(STARTING_URL, MAX_PAGES_TO_CRAWL, numThreads).run();
    }
}
