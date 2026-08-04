/// <reference types="vite/client" />

interface ImportMetaEnv {
  // Base pública del object storage, para mostrar las fotos ya subidas.
  // Se fija en tiempo de build (build-arg del Dockerfile del gateway) porque
  // el panel es un bundle estático sin configuración en runtime.
  readonly VITE_S3_PUBLIC_BASE?: string;
}

interface ImportMeta {
  readonly env: ImportMetaEnv;
}
