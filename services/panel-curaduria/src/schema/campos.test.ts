import { describe, expect, it } from 'vitest';
import {
  atributosParaEnviar,
  camposDeSchema,
  conValorEn,
  etiquetaDeClave,
  faltantesRequeridos,
  valorEn,
} from './campos';
import type { JsonSchema } from '../api/tipos';

// Recorte del schema real de Fungi (services/especies-api/config/schemas/fungi.json).
// La forma importa más que la exhaustividad: enum obligatorio, array de enums,
// texto largo y un tipo que el panel no sabe dibujar.
const schemaFungi: JsonSchema = {
  type: 'object',
  additionalProperties: false,
  required: ['comestibilidad'],
  properties: {
    comestibilidad: { type: 'string', enum: ['comestible', 'toxico', 'desconocido'] },
    sustrato: { type: 'array', uniqueItems: true, items: { type: 'string', enum: ['suelo', 'musgo'] } },
    advertencia: { type: 'string', maxLength: 2000 },
    nombre_corto: { type: 'string', maxLength: 60 },
    tuplas: { type: 'array', items: { type: 'object' } },
  },
};

const schemaPlantae: JsonSchema = {
  type: 'object',
  properties: {
    altura_promedio_m: { type: 'number', minimum: 0, maximum: 200 },
    floracion_meses: { type: 'array', items: { type: 'integer', minimum: 1, maximum: 12 } },
    tipo_hoja: {
      type: 'object',
      required: ['ciclo'],
      properties: {
        ciclo: { type: 'string', enum: ['perenne', 'caduca'] },
        morfologia: { type: 'string', enum: ['simple', 'compuesta'] },
      },
    },
  },
};

describe('camposDeSchema', () => {
  it('mapea cada forma del schema a su control', () => {
    const campos = camposDeSchema(schemaFungi);
    const porClave = Object.fromEntries(campos.map((campo) => [campo.clave, campo]));

    expect(porClave.comestibilidad?.tipo).toBe('seleccion');
    expect(porClave.comestibilidad?.requerido).toBe(true);
    expect(porClave.comestibilidad?.opciones).toEqual(['comestible', 'toxico', 'desconocido']);

    expect(porClave.sustrato?.tipo).toBe('multiseleccion');
    expect(porClave.sustrato?.opciones).toEqual(['suelo', 'musgo']);
    expect(porClave.sustrato?.requerido).toBe(false);

    // El umbral de maxLength decide input de una línea vs textarea.
    expect(porClave.advertencia?.tipo).toBe('texto-largo');
    expect(porClave.nombre_corto?.tipo).toBe('texto');
  });

  it('marca como no soportado lo que no sabe dibujar en vez de esconderlo', () => {
    const campos = camposDeSchema(schemaFungi);
    expect(campos.find((campo) => campo.clave === 'tuplas')?.tipo).toBe('no-soportado');
  });

  it('expande un array de enteros acotados en opciones', () => {
    const meses = camposDeSchema(schemaPlantae).find((c) => c.clave === 'floracion_meses');
    expect(meses?.tipo).toBe('multiseleccion');
    expect(meses?.opciones).toHaveLength(12);
    expect(meses?.opciones?.[0]).toBe(1);
    expect(meses?.opciones?.[11]).toBe(12);
  });

  it('anida los objetos con la ruta completa', () => {
    const tipoHoja = camposDeSchema(schemaPlantae).find((c) => c.clave === 'tipo_hoja');
    expect(tipoHoja?.tipo).toBe('objeto');
    expect(tipoHoja?.hijos?.map((hijo) => hijo.ruta)).toEqual([
      ['tipo_hoja', 'ciclo'],
      ['tipo_hoja', 'morfologia'],
    ]);
    expect(tipoHoja?.hijos?.find((hijo) => hijo.clave === 'ciclo')?.requerido).toBe(true);
  });

  it('deriva etiquetas legibles del snake_case', () => {
    expect(etiquetaDeClave('usos_tradicionales')).toBe('Usos tradicionales');
  });
});

describe('lectura y escritura por ruta', () => {
  it('escribe en profundidad sin mutar el original', () => {
    const original = { tipo_hoja: { ciclo: 'perenne' } };
    const nuevo = conValorEn(original, ['tipo_hoja', 'morfologia'], 'simple');

    expect(valorEn(nuevo, ['tipo_hoja', 'morfologia'])).toBe('simple');
    expect(valorEn(nuevo, ['tipo_hoja', 'ciclo'])).toBe('perenne');
    expect(original).toEqual({ tipo_hoja: { ciclo: 'perenne' } });
  });

  it('crea los niveles intermedios que falten', () => {
    expect(conValorEn({}, ['fruto', 'comestible'], 'toxico')).toEqual({
      fruto: { comestible: 'toxico' },
    });
  });

  it('devuelve undefined en rutas que no existen', () => {
    expect(valorEn({ a: 1 }, ['a', 'b'])).toBeUndefined();
    expect(valorEn({}, ['x'])).toBeUndefined();
  });
});

describe('atributosParaEnviar', () => {
  it('poda vacíos que el schema estricto rechazaría', () => {
    const limpio = atributosParaEnviar({
      comestibilidad: 'toxico',
      advertencia: '',
      sustrato: [],
      tipo_hoja: { ciclo: '', morfologia: '' },
      altura_promedio_m: 0,
      endemica: false,
    });

    expect(limpio).toEqual({ comestibilidad: 'toxico', altura_promedio_m: 0, endemica: false });
  });

  it('un formulario intacto se envía como objeto vacío, no como null', () => {
    expect(atributosParaEnviar({ tipo_hoja: {} })).toEqual({});
  });
});

describe('faltantesRequeridos', () => {
  it('detecta el enum obligatorio de Fungi sin elegir', () => {
    const campos = camposDeSchema(schemaFungi);
    expect(faltantesRequeridos(campos, {})).toEqual(['Comestibilidad']);
    expect(faltantesRequeridos(campos, { comestibilidad: 'toxico' })).toEqual([]);
  });

  it('mira también los requeridos anidados', () => {
    const campos = camposDeSchema(schemaPlantae);
    expect(faltantesRequeridos(campos, {})).toEqual(['Ciclo']);
    expect(faltantesRequeridos(campos, { tipo_hoja: { ciclo: 'caduca' } })).toEqual([]);
  });
});
