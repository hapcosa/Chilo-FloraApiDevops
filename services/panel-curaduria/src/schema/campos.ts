import type { JsonSchema } from '../api/tipos';

// Traducción de un JSON Schema de `atributos_especificos` a una lista de campos
// renderizables. Los schemas viven en el servidor (GET /api/v1/schemas/:reino);
// aquí solo se decide qué control dibuja cada tipo.
//
// Lo que no encaje en estos tipos se marca 'no-soportado' y se muestra como
// aviso: es preferible que el curador vea que un campo no se puede editar desde
// el panel a que desaparezca sin dejar rastro.
export type CampoTipo =
  | 'texto'
  | 'texto-largo'
  | 'numero'
  | 'entero'
  | 'seleccion'
  | 'multiseleccion'
  | 'objeto'
  | 'no-soportado';

export interface Campo {
  clave: string;
  ruta: string[];
  etiqueta: string;
  descripcion?: string;
  requerido: boolean;
  tipo: CampoTipo;
  opciones?: (string | number)[];
  minimo?: number;
  maximo?: number;
  maxLength?: number;
  hijos?: Campo[];
}

// `usos_tradicionales` -> `Usos tradicionales`. Los schemas usan snake_case y no
// traen `title` por propiedad, así que se deriva de la clave.
export function etiquetaDeClave(clave: string): string {
  const conEspacios = clave.replace(/_/g, ' ');
  return conEspacios.charAt(0).toUpperCase() + conEspacios.slice(1);
}

// Un array de enums se edita como multiselección; un array de enteros acotados
// (los meses de floración) también, generando las opciones del rango.
function opcionesDeItems(items: JsonSchema | undefined): (string | number)[] | undefined {
  if (!items) return undefined;
  if (items.enum) return items.enum;
  if (
    items.type === 'integer' &&
    typeof items.minimum === 'number' &&
    typeof items.maximum === 'number' &&
    items.maximum - items.minimum <= 100
  ) {
    const opciones: number[] = [];
    for (let valor = items.minimum; valor <= items.maximum; valor += 1) opciones.push(valor);
    return opciones;
  }
  return undefined;
}

function campoDe(clave: string, schema: JsonSchema, ruta: string[], requerido: boolean): Campo {
  const base = {
    clave,
    ruta,
    etiqueta: schema.title ?? etiquetaDeClave(clave),
    descripcion: schema.description,
    requerido,
  };

  if (schema.enum) {
    return { ...base, tipo: 'seleccion', opciones: schema.enum };
  }

  switch (schema.type) {
    case 'string':
      return {
        ...base,
        // El umbral separa un nombre corto de un texto de advertencia; sale de
        // los maxLength que ya usan los schemas (1000-2000 para texto libre).
        tipo: (schema.maxLength ?? 0) > 300 ? 'texto-largo' : 'texto',
        maxLength: schema.maxLength,
      };
    case 'number':
    case 'integer':
      return {
        ...base,
        tipo: schema.type === 'integer' ? 'entero' : 'numero',
        minimo: schema.minimum,
        maximo: schema.maximum,
      };
    case 'array': {
      const opciones = opcionesDeItems(schema.items);
      if (!opciones) return { ...base, tipo: 'no-soportado' };
      return { ...base, tipo: 'multiseleccion', opciones };
    }
    case 'object':
      return {
        ...base,
        tipo: 'objeto',
        hijos: camposDeSchema(schema, ruta),
      };
    default:
      return { ...base, tipo: 'no-soportado' };
  }
}

export function camposDeSchema(schema: JsonSchema, ruta: string[] = []): Campo[] {
  if (!schema.properties) return [];
  const requeridos = new Set(schema.required ?? []);
  return Object.entries(schema.properties).map(([clave, sub]) =>
    campoDe(clave, sub, [...ruta, clave], requeridos.has(clave)),
  );
}

// ----- lectura y escritura por ruta -----

export function valorEn(objeto: Record<string, unknown>, ruta: string[]): unknown {
  let actual: unknown = objeto;
  for (const paso of ruta) {
    if (typeof actual !== 'object' || actual === null) return undefined;
    actual = (actual as Record<string, unknown>)[paso];
  }
  return actual;
}

export function conValorEn(
  objeto: Record<string, unknown>,
  ruta: string[],
  valor: unknown,
): Record<string, unknown> {
  const [cabeza, ...resto] = ruta;
  if (cabeza === undefined) return objeto;
  if (resto.length === 0) return { ...objeto, [cabeza]: valor };

  const hijoActual = objeto[cabeza];
  const hijo =
    typeof hijoActual === 'object' && hijoActual !== null
      ? (hijoActual as Record<string, unknown>)
      : {};
  return { ...objeto, [cabeza]: conValorEn(hijo, resto, valor) };
}

// El schema declara `additionalProperties: false` y tipos estrictos: mandar
// `""` donde se espera un enum, o un objeto vacío donde no se tocó nada, hace
// fallar la validación del servidor. Se poda antes de enviar.
export function limpiarVacios(valor: unknown): unknown {
  if (Array.isArray(valor)) {
    const items = valor.map(limpiarVacios).filter((item) => item !== undefined);
    return items.length > 0 ? items : undefined;
  }
  if (typeof valor === 'object' && valor !== null) {
    const salida: Record<string, unknown> = {};
    for (const [clave, sub] of Object.entries(valor as Record<string, unknown>)) {
      const limpio = limpiarVacios(sub);
      if (limpio !== undefined) salida[clave] = limpio;
    }
    return Object.keys(salida).length > 0 ? salida : undefined;
  }
  if (valor === '' || valor === null || valor === undefined) return undefined;
  return valor;
}

export function atributosParaEnviar(atributos: Record<string, unknown>): Record<string, unknown> {
  return (limpiarVacios(atributos) as Record<string, unknown> | undefined) ?? {};
}

// Validación local mínima: solo lo que el schema declara `required`. El resto
// lo dice el servidor, que es quien manda. Sirve para no gastar un viaje —y
// para que el curador de Fungi no pueda olvidarse de la comestibilidad.
export function faltantesRequeridos(
  campos: Campo[],
  atributos: Record<string, unknown>,
): string[] {
  const faltantes: string[] = [];
  for (const campo of campos) {
    if (campo.requerido) {
      const valor = valorEn(atributos, campo.ruta);
      const vacio =
        valor === undefined || valor === null || valor === '' ||
        (Array.isArray(valor) && valor.length === 0);
      if (vacio) faltantes.push(campo.etiqueta);
    }
    if (campo.hijos) faltantes.push(...faltantesRequeridos(campo.hijos, atributos));
  }
  return faltantes;
}
