import { parentPort } from 'worker_threads';
import * as cheerio from 'cheerio';
import { URL } from 'url';

function parseHtml(html, pageUrl, baseHostname) {
    const $ = cheerio.load(html);

    // 1. Process data (CPU-bound task)
    const tableRows = $('table tr');
    let totalClose = 0;
    tableRows.slice(1).each((i, row) => {
        const cells = $(row).find('td');
        if (cells.length > 4) {
            totalClose += parseFloat($(cells).eq(4).text());
        }
    });

    // 2. Extract next link
    const nextLinkTag = $('a').filter((i, el) => $(el).text() === 'Next');
    let nextUrl = null;
    if (nextLinkTag.length > 0) {
        const nextHref = nextLinkTag.attr('href');
        const absoluteUrl = new URL(nextHref, pageUrl).href;
        if (new URL(absoluteUrl).hostname === baseHostname) {
            nextUrl = absoluteUrl;
        }
    }
    
    return { nextUrl };
}

parentPort.on('message', ({ html, pageUrl, baseHostname }) => {
    const result = parseHtml(html, pageUrl, baseHostname);
    parentPort.postMessage(result);
});
