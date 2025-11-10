import axios from 'axios';
import { Worker } from 'worker_threads';
import { URL } from 'url';
import path from 'path';
import os from 'os';

// --- Configuration ---
const STARTING_URL = "http://192.168.0.141:5000/page_0.html";
const MAX_PAGES_TO_CRAWL = 20;
// --------------------

// Use a command-line argument for thread count, or default to CPU cores
const NUM_THREADS = process.argv[2] ? parseInt(process.argv[2]) : os.cpus().length;

class WorkerPool {
    constructor(filePath, numThreads) {
        this.workers = [];
        this.freeWorkers = [];
        for (let i = 0; i < numThreads; i++) {
            const worker = new Worker(filePath);
            this.workers.push(worker);
            this.freeWorkers.push(worker);
        }
    }

    run(task) {
        return new Promise((resolve, reject) => {
            if (this.freeWorkers.length === 0) {
                // This shouldn't happen with our logic, but it's a safeguard
                return reject(new Error("No free workers"));
            }

            const worker = this.freeWorkers.pop();
            worker.once('message', (result) => {
                this.freeWorkers.push(worker);
                resolve(result);
            });
            worker.once('error', (err) => {
                this.freeWorkers.push(worker);
                reject(err);
            });
            worker.postMessage(task);
        });
    }

    terminate() {
        this.workers.forEach(worker => worker.terminate());
    }
}

async function main() {
    const startTime = process.hrtime.bigint();
    const visitedUrls = new Set([STARTING_URL]);
    const frontier = [STARTING_URL];
    const baseHostname = new URL(STARTING_URL).hostname;
    let pagesCrawled = 0;

    const workerFile = path.resolve(process.cwd(), 'javascript/parser_worker.js');
    const pool = new WorkerPool(workerFile, NUM_THREADS);

    let activeRequests = 0;
    
    while (pagesCrawled < MAX_PAGES_TO_CRAWL && frontier.length > 0) {
        const urlsToProcess = frontier.splice(0, NUM_THREADS - activeRequests);
        
        const promises = urlsToProcess.map(async (url) => {
            activeRequests++;
            try {
                const response = await axios.get(url, { timeout: 10000 });
                pagesCrawled++;
                
                const result = await pool.run({ 
                    html: response.data, 
                    pageUrl: url,
                    baseHostname: baseHostname 
                });

                if (result.nextUrl && !visitedUrls.has(result.nextUrl)) {
                    visitedUrls.add(result.nextUrl);
                    frontier.push(result.nextUrl);
                }
            } catch (error) {
                // Ignore failed fetches
            } finally {
                activeRequests--;
            }
        });
        await Promise.all(promises);
    }
    
    pool.terminate();

    const endTime = process.hrtime.bigint();
    const totalTimeS = Number(endTime - startTime) / 1e9;

    const results = {
        language: "JavaScript",
        version: "Worker Threads",
        threads: NUM_THREADS,
        pages_crawled: pagesCrawled,
        total_time_s: parseFloat(totalTimeS.toFixed(4)),
        pages_per_second: parseFloat((pagesCrawled / totalTimeS).toFixed(2))
    };
    console.log(JSON.stringify(results));
}

main();
