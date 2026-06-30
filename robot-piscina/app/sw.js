// sw.js — service worker dell'app di controllo (PWA installabile, uso offline).
const CACHE = "robot-piscina-v1";
const ASSETS = [
  "./",
  "./index.html",
  "./manifest.json",
  "./icon-192.png",
  "./icon-512.png"
];

self.addEventListener("install", (e) => {
  e.waitUntil(caches.open(CACHE).then((c) => c.addAll(ASSETS)).then(() => self.skipWaiting()));
});

self.addEventListener("activate", (e) => {
  e.waitUntil(
    caches.keys().then((keys) =>
      Promise.all(keys.filter((k) => k !== CACHE).map((k) => caches.delete(k)))
    ).then(() => self.clients.claim())
  );
});

self.addEventListener("fetch", (e) => {
  const req = e.request;
  // Solo richieste GET http(s): le connessioni WebSocket al robot non passano di qui.
  if (req.method !== "GET" || !req.url.startsWith("http")) return;
  // Cache-first sugli asset dell'app shell; rete come fallback.
  e.respondWith(
    caches.match(req).then((hit) => hit || fetch(req).catch(() => caches.match("./index.html")))
  );
});
