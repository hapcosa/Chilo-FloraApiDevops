import { construirQuery, request, type RespuestaLista } from './client';
import type {
  Avistamiento,
  Categoria,
  Especie,
  EspecieEstado,
  Familia,
  Genero,
  JsonSchema,
  Postulacion,
  PostulacionEstado,
  PresignedUpload,
  Reino,
  RespuestaLogin,
  Usuario,
} from './tipos';

export interface FiltrosEspecies {
  categoria_id?: number;
  estado?: EspecieEstado | '';
  reino?: Reino | '';
  q?: string;
  limit?: number;
  offset?: number;
}

// Fábrica en vez de módulo con estado: el token cambia al entrar y salir, y así
// ninguna pantalla puede olvidarse de pasarlo.
export function crearApi(token: string | null, onNoAutorizado: () => void) {
  const pedir = <T>(path: string, init: { method?: string; body?: unknown } = {}) =>
    request<T>(path, { ...init, token, onNoAutorizado });

  return {
    // ----- auth -----
    perfil: () => pedir<Usuario>('/api/v1/auth/me'),

    // ----- catálogo -----
    categorias: (reino?: Reino) =>
      pedir<RespuestaLista<Categoria>>(`/api/v1/categorias${construirQuery({ reino })}`).then(
        (r) => r.data,
      ),

    categoriasDe: (usuarioId: number) =>
      pedir<RespuestaLista<Categoria>>(`/api/v1/moderadores/${usuarioId}/categorias`).then(
        (r) => r.data,
      ),

    familias: () => pedir<RespuestaLista<Familia>>('/api/familias').then((r) => r.data),

    generos: () => pedir<RespuestaLista<Genero>>('/api/generos').then((r) => r.data),

    schema: (reino: Reino) => pedir<JsonSchema>(`/api/v1/schemas/${reino}`),

    // ----- especies -----
    especies: (filtros: FiltrosEspecies) =>
      pedir<RespuestaLista<Especie>>(`/api/v1/especies${construirQuery({ ...filtros })}`),

    especie: (id: number) => pedir<Especie>(`/api/v1/especies/${id}`),

    crearEspecie: (especie: Partial<Especie>) =>
      pedir<Especie>('/api/v1/especies', { method: 'POST', body: especie }),

    actualizarEspecie: (id: number, especie: Partial<Especie>) =>
      pedir<Especie>(`/api/v1/especies/${id}`, { method: 'PUT', body: especie }),

    guardarFotos: (id: number, fotos: { foto_portada_key: string | null; fotos_keys: string[] }) =>
      pedir<Especie>(`/api/v1/especies/${id}/fotos`, { method: 'PATCH', body: fotos }),

    publicar: (id: number) => pedir<Especie>(`/api/v1/especies/${id}/publicar`, { method: 'POST' }),

    despublicar: (id: number) =>
      pedir<Especie>(`/api/v1/especies/${id}/despublicar`, { method: 'POST' }),

    // ----- postulaciones -----
    postulaciones: (estado?: PostulacionEstado) =>
      pedir<RespuestaLista<Postulacion>>(`/api/v1/postulaciones${construirQuery({ estado })}`).then(
        (r) => r.data,
      ),

    resolverPostulacion: (id: number, estado: 'aprobada' | 'rechazada', motivo?: string) =>
      pedir<Postulacion>(`/api/v1/postulaciones/${id}`, {
        method: 'PATCH',
        body: estado === 'rechazada' ? { estado, motivo } : { estado },
      }),

    // ----- avistamientos -----
    avistamientos: (params: { estado?: string; reino?: Reino | ''; limit?: number; offset?: number }) =>
      pedir<RespuestaLista<Avistamiento>>(`/api/v1/avistamientos${construirQuery({ ...params })}`),

    moderarAvistamiento: (id: number, estado: 'aprobado' | 'rechazado', motivo?: string) =>
      pedir<Avistamiento>(`/api/v1/avistamientos/${id}/moderacion`, {
        method: 'PATCH',
        body: estado === 'rechazado' ? { estado, motivo_rechazo: motivo } : { estado },
      }),

    // ----- fotos -----
    presign: (bucket: string, filename: string, contentType: string) =>
      pedir<PresignedUpload>('/api/v1/uploads/presign', {
        method: 'POST',
        body: { bucket, filename, content_type: contentType },
      }),
  };
}

export type Api = ReturnType<typeof crearApi>;

// Login vive fuera de la fábrica: es lo único que se llama sin token.
export function login(email: string, password: string): Promise<RespuestaLogin> {
  return request<RespuestaLogin>('/api/v1/auth/login', {
    method: 'POST',
    body: { email, password },
  });
}

// Login con Google: el panel manda el `credential` (el ID token que devuelve
// Google Identity Services) y el auth-service lo verifica contra Google y emite
// el JWT propio. Mismo endpoint que usa la app móvil.
export function loginConGoogle(credential: string): Promise<RespuestaLogin> {
  return request<RespuestaLogin>('/api/v1/auth/google', {
    method: 'POST',
    body: { credential },
  });
}

// La subida va directa al object storage con la URL presignada: nunca
// multipart contra la API (ver CLAUDE.md § Fotos).
export async function subirArchivo(presign: PresignedUpload, archivo: File): Promise<void> {
  const respuesta = await fetch(presign.url, {
    method: presign.method,
    headers: presign.headers,
    body: archivo,
  });
  if (!respuesta.ok) {
    throw new Error(`El object storage rechazó la subida (${respuesta.status})`);
  }
}
