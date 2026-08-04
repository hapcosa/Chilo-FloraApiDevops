// Espejo de los contratos JSON de especies-api y auth-service. Solo los campos
// que el panel usa: ampliar aquí cuando se necesite alguno más.

export const REINOS = ['animalia', 'plantae', 'fungi', 'protista', 'monera'] as const;
export type Reino = (typeof REINOS)[number];

export type EspecieEstado = 'borrador' | 'publicada';

export interface Usuario {
  id: number;
  email: string;
  name: string;
  role: 'admin' | 'moderator' | 'researcher' | 'user';
}

export interface RespuestaLogin {
  user: Usuario;
  access_token: string;
  refresh_token: string;
  expires_in: number;
}

export interface Categoria {
  id: number;
  slug: string;
  nombre: string;
  reino: Reino;
  descripcion: string | null;
}

export interface Especie {
  id: number;
  reino: Reino;
  genero_id: number;
  genero_nombre?: string;
  nombre_cientifico: string;
  nombre_comun: string;
  autor_cientifico: string;
  descripcion: string;
  habitat: string;
  distribucion_chiloe: string;
  endemica: boolean;
  estado_conservacion: string;
  fuentes: unknown[];
  geo_lat: number | null;
  geo_lng: number | null;
  atributos_especificos: Record<string, unknown>;
  foto_portada_key: string | null;
  fotos_keys: string[];
  categoria_id: number | null;
  estado: EspecieEstado;
  publicado_por: number | null;
  fecha_publicacion: string | null;
  created_at: string | null;
  updated_at: string | null;
}

export interface Familia {
  id: number;
  nombre: string;
  reino: Reino;
}

export interface Genero {
  id: number;
  nombre: string;
  familia_id: number;
}

export type PostulacionEstado = 'pendiente' | 'aprobada' | 'rechazada';

export interface Postulacion {
  id: number;
  usuario_id: number;
  categoria_id: number;
  texto: string;
  estado: PostulacionEstado;
  revisado_por: number | null;
  revisado_en: string | null;
  motivo: string | null;
  created_at: string | null;
}

export type AvistamientoEstado = 'pendiente' | 'aprobado' | 'rechazado';

export interface Avistamiento {
  id: number;
  especie_id: number | null;
  reino: Reino;
  nombre_sugerido: string | null;
  descripcion: string | null;
  foto_key: string;
  geo_lat: number;
  geo_lng: number;
  observado_en: string | null;
  creado_por: number | null;
  estado: AvistamientoEstado;
  motivo_rechazo: string | null;
  created_at: string | null;
}

export interface PresignedUpload {
  success: boolean;
  method: string;
  bucket: string;
  key: string;
  url: string;
  headers: Record<string, string>;
  expires_in: number;
}

// Subconjunto de JSON Schema que el formulario sabe renderizar. Lo que no
// encaje aquí se muestra como aviso en vez de silenciarse.
export interface JsonSchema {
  type?: string;
  title?: string;
  description?: string;
  enum?: (string | number)[];
  required?: string[];
  properties?: Record<string, JsonSchema>;
  items?: JsonSchema;
  minimum?: number;
  maximum?: number;
  maxLength?: number;
  uniqueItems?: boolean;
  additionalProperties?: boolean;
}
