// Vigneto Piediluco — Service Worker
// Cache-first per assets statici, network-first per Supabase

const CACHE = 'vigneto-v7';
const STATIC = [
  '/',
  '/index.html',
  'https://fonts.googleapis.com/css2?family=Doto:ROND,wght@0,100..900;1,100..900&family=Space+Mono:wght@400;700&family=Space+Grotesk:wght@300;400;500;600;700&display=swap'
];

// Installa e pre-cacha gli asset statici
self.addEventListener('install', e => {
  e.waitUntil(
    caches.open(CACHE).then(cache => cache.addAll(STATIC)).then(() => self.skipWaiting())
  );
});

// Rimuovi vecchie cache
self.addEventListener('activate', e => {
  e.waitUntil(
    caches.keys().then(keys =>
      Promise.all(keys.filter(k => k !== CACHE).map(k => caches.delete(k)))
    ).then(() => self.clients.claim())
  );
});

// Strategia: Supabase → network only | tutto il resto → cache first
self.addEventListener('fetch', e => {
  const url = new URL(e.request.url);

  // Supabase: sempre network, mai cache (dati live)
  if (url.hostname.includes('supabase.co')) {
    e.respondWith(fetch(e.request).catch(() => new Response('{"error":"offline"}', { headers: { 'Content-Type': 'application/json' } })));
    return;
  }

  // Google Fonts: cache first
  if (url.hostname.includes('fonts.')) {
    e.respondWith(
      caches.match(e.request).then(cached => cached || fetch(e.request).then(res => {
        const clone = res.clone();
        caches.open(CACHE).then(c => c.put(e.request, clone));
        return res;
      }))
    );
    return;
  }

  // App shell (index.html, icon): cache first, fallback network
  e.respondWith(
    caches.match(e.request).then(cached => {
      if (cached) return cached;
      return fetch(e.request).then(res => {
        if (res.ok) {
          const clone = res.clone();
          caches.open(CACHE).then(c => c.put(e.request, clone));
        }
        return res;
      });
    })
  );
});
