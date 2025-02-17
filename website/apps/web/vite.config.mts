import react from '@vitejs/plugin-react'

import path from 'path'
import { defineConfig } from 'vite'

// https://vitejs.dev/config/
export default defineConfig({
  base: './',
  resolve: {
    alias: {
      '@': path.join(__dirname, 'src'),
    },
  },
  plugins: [react()],
  server: {
    port: 3000,
  },
  build: {
    sourcemap: true,
  },
})
