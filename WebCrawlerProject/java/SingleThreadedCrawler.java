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
import java.util.concurrent.TimeUnit;

public class SingleThreadedCrawler {
    private static final String STARTING_URL = "http://192.168.0.141:5000/page_0.html";
    private static final int MAX_PAGES_TO_CRAWL = 20;

    private final int maxPages;
    private final HttpClient httpClient;
    private final Gson gson;
    private final String baseHost;

    private final Set<String> visitedUrls = new HashSet<>();
    private final Queue<String> frontier = new LinkedList<>();
    private int pagesCrawled = 0;

    public SingleThreadedCrawler(String startUrl, int maxPages) throws URISyntaxException {
        this.maxPages = maxPages;
        this.httpClient = HttpClient.newBuilder().version(HttpClient.Version.HTTP_1_1).build();
        this.gson = new Gson();
        this.baseHost = new URI(startUrl).getHost();
        this.frontier.add(startUrl);
    }

    public void run() {
        long startTime = System.nanoTime();

        while (!frontier.isEmpty() && pagesCrawled < maxPages) {
            String url = frontier.poll();
            if (visitedUrls.contains(url)) {
                continue;
            }

            try {
                HttpRequest request = HttpRequest.newBuilder().uri(new URI(url)).GET().build();
                HttpResponse<String> response = httpClient.send(request, HttpResponse.BodyHandlers.ofString());

                if (response.statusCode() == 200) {
                    visitedUrls.add(url);
                    pagesCrawled++;
                    Document doc = Jsoup.parse(response.body(), url);
                    
                    // REVISED: Process data from the table
                    processPageData(doc);
                    
                    // REVISED: Find the single "Next" link
                    Element nextLink = doc.selectFirst("a:contains(Next)");
                    if (nextLink != null) {
                        String absUrl = nextLink.absUrl("href");
                        if (new URI(absUrl).getHost().equals(baseHost) && !visitedUrls.contains(absUrl)) {
                            frontier.add(absUrl);
                        }
                    }
                }
            } catch (Exception e) {
                // Ignore failed pages
            }
        }

        long endTime = System.nanoTime();
        double totalTimeSec = TimeUnit.NANOSECONDS.toMillis(endTime - startTime) / 1000.0;

        Map<String, Object> results = new LinkedHashMap<>();
        results.put("language", "Java");
        results.put("version", "Single-Threaded");
        results.put("threads", 1);
        results.put("pages_crawled", pagesCrawled);
        results.put("total_time_s", Math.round(totalTimeSec * 10000.0) / 10000.0);
        results.put("pages_per_second", Math.round((pagesCrawled / totalTimeSec) * 100.0) / 100.0);

        System.out.println(gson.toJson(results));
    }

    private void processPageData(Document doc) {
        try {
            Elements rows = doc.select("table tr");
            if (rows.size() < 2) return; // Must have at least a header and one data row

            double totalClose = 0;
            // Iterate from the second row (index 1) to skip the header
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

    public static void main(String[] args) throws URISyntaxException {
        new SingleThreadedCrawler(STARTING_URL, MAX_PAGES_TO_CRAWL).run();
    }
}
