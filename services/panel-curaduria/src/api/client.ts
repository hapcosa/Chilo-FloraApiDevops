// Cliente HTTP único del panel. El panel se sirve desde el mismo origen que el
// gateway, así que las rutas son relativas y el JWT viaja en Authorization.

export class ApiError extends Error {
  constructor(
    readonly status: number,
    message: string,
  ) {
    super(message);
    this.name = 'ApiError';
  }
}

// La API no tiene una forma de error única: unas rutas devuelven {"error": ...},
// otras {"message": ...} y el gateway inyecta las suyas. Se prueban todas antes
// de caer en un genérico, porque el mensaje del servidor suele ser el único
// dato útil para el curador (ej. qué campo no cumple el JSON Schema).
function mensajeDeError(status: number, cuerpo: unknown): string {
  if (typeof cuerpo === 'object' && cuerpo !== null) {
    const registro = cuerpo as Record<string, unknown>;
    for (const clave of ['error', 'message', 'details']) {
      const valor = registro[clave];
      if (typeof valor === 'string' && valor.length > 0) return valor;
    }
  }
  if (status === 401) return 'Sesión expirada o inválida.';
  if (status === 403) return 'No tienes curaduría sobre esa categoría.';
  return `Error ${status}`;
}

export interface RequestOptions {
  method?: string;
  body?: unknown;
  token?: string | null;
  // Se invoca cuando el servidor responde 401, para que la sesión se cierre en
  // un solo sitio en vez de en cada pantalla.
  onNoAutorizado?: () => void;
}

export async function request<T>(path: string, options: RequestOptions = {}): Promise<T> {
  const { method = 'GET', body, token, onNoAutorizado } = options;

  const headers: Record<string, string> = {};
  if (body !== undefined) headers['Content-Type'] = 'application/json';
  if (token) headers['Authorization'] = `Bearer ${token}`;

  const response = await fetch(path, {
    method,
    headers,
    body: body === undefined ? undefined : JSON.stringify(body),
  });

  const texto = await response.text();
  let cuerpo: unknown = null;
  if (texto.length > 0) {
    try {
      cuerpo = JSON.parse(texto);
    } catch {
      cuerpo = texto;
    }
  }

  if (!response.ok) {
    if (response.status === 401) onNoAutorizado?.();
    throw new ApiError(response.status, mensajeDeError(response.status, cuerpo));
  }

  return cuerpo as T;
}

// Varios listados responden {success, data, pagination} y los recursos sueltos
// responden el objeto pelado. Este helper normaliza el primer caso.
export interface RespuestaLista<T> {
  success: boolean;
  data: T[];
  pagination?: { limit: number; offset: number; total: number };
}

export function construirQuery(params: Record<string, string | number | undefined | null>): string {
  const query = new URLSearchParams();
  for (const [clave, valor] of Object.entries(params)) {
    if (valor === undefined || valor === null || valor === '') continue;
    query.set(clave, String(valor));
  }
  const serializada = query.toString();
  return serializada.length > 0 ? `?${serializada}` : '';
}
