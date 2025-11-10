import axios from 'axios';
import * as cheerio from 'cheerio';
import { URL } from 'url';

// --- Configuration ---
const STARTING_URL = "http://192.168.0.141:5000/page_0.html";
const MAX_PAGES_TO_CRAWL = 20;
const CONCURRENT_REQUESTS = 10; // How many requests to run in parallel
// --------------------

const visitedUrls = new Set();
const frontier = [STARTING_URL];
const baseHostname = new URL(STARTING_URL).hostname;
let pagesCrawled = 0;

function processPageData($) {
    const tableRows = $('table tr');
    let totalClose = 0;
    tableRows.slice(1).each((i, row) => {
        const cells = $(row).find('td');
        if (cells.length > 4) {
            const closePriceText = $(cells).eq(4).text();
            totalClose += parseFloat(closePriceText);
        }
    });
}

async function crawl(url) {
    if (pagesCrawled >= MAX_PAGES_TO_CRAWL || visitedUrls.has(url)) {
        return;
    }
    visitedUrls.add(url);

    try {
        const response = await axios.get(url, { timeout: 10000 });
        pagesCrawled++;
        const $ = cheerio.load(response.data);

        processPageData($);

        const nextLinkTag = $('a').filter((i, el) => $(el).text() === 'Next');
        if (nextLinkTag.length > 0) {
            const nextHref = nextLinkTag.attr('href');
            const absoluteUrl = new URL(nextHref, url).href;
            if (new URL(absoluteUrl).hostname === baseHostname) {
                frontier.push(absoluteUrl);
            }
        }
    } catch (error) {
        // console.error(`Failed to crawl ${url}: ${error.message}`);
    }
}

async function main() {
    const startTime = process.hrtime.bigint();
    let activeRequests = 0;

    const run = async () => {
        while (pagesCrawled < MAX_PAGES_TO_CRAWL && (frontier.length > 0 || activeRequests > 0)) {
            while (frontier.length > 0 && activeRequests < CONCURRENT_REQUESTS) {
                activeRequests++;
                const url = frontier.shift();
                crawl(url).finally(() => activeRequests--);
            }
            // A small delay to prevent a tight loop if all requests are active
            if (activeRequests > 0) {
                 await new Promise(resolve => setTimeout(resolve, 5));
            }
        }
    };

    await run();

    const endTime = process.hrtime.bigint();
    const totalTimeMs = Number(endTime - startTime) / 1e6;
    const totalTimeS = totalTimeMs / 1000;

    const results = {
        language: "JavaScript",
        version: "Async Baseline",
        threads: 1, // Event loop is on a single main thread
        pages_crawled: pagesCrawled,
        total_time_s: parseFloat(totalTimeS.toFixed(4)),
        pages_per_second: parseFloat((pagesCrawled / totalTimeS).toFixed(2))
    };
    console.log(JSON.stringify(results));
}

main();
