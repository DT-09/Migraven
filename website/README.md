# Migraven website

Static, multi-page product site for Migraven. Cloudflare Pages can serve the contents of this directory directly; no build step is required.

## Pages
- `index.html` — product positioning, target users, workflow, benefits and commercial path
- `product.html` — capabilities, inputs, engine rules and deliverables
- `demo.html` — local browser-side CSV comparison demonstration
- `method.html` — comparison semantics and evidence boundaries
- `evaluation.html` — bounded paid-evaluation scope and request-draft workflow
- `faq.html` — product, data and commercial questions

## Demo privacy
The browser demo parses selected CSV files in the current tab. It does not upload, persist or transmit file contents. The demo is a JavaScript demonstration of the documented exact-match rules; the production CLI and comparison library remain implemented in C17.

## Deployment
Set the Cloudflare Pages output directory to `website`. Leave the build command empty.
