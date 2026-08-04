import { defineConfig } from 'vitest/config';
import react from '@vitejs/plugin-react';

// El panel se sirve bajo /curaduria/ desde el propio contenedor del gateway,
// así que las llamadas a /api/... van al mismo origen y no hay CORS ni una
// URL de API que configurar en runtime.
export default defineConfig({
  base: '/curaduria/',
  plugins: [react()],
  server: {
    port: 5173,
    // En `npm run dev` el panel corre fuera del gateway: se proxean las rutas
    // de API para reproducir el mismo-origen de producción.
    proxy: {
      '/api': {
        target: process.env.PANEL_API_TARGET ?? 'http://localhost:8080',
        changeOrigin: true,
      },
    },
  },
  test: {
    environment: 'node',
    include: ['src/**/*.test.ts'],
  },
});
