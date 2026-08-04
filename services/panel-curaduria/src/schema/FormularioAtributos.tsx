import { Aviso } from '../componentes/Aviso';
import { conValorEn, valorEn, type Campo } from './campos';

interface Props {
  campos: Campo[];
  atributos: Record<string, unknown>;
  onChange: (atributos: Record<string, unknown>) => void;
}

function ControlCampo({ campo, atributos, onChange }: { campo: Campo } & Omit<Props, 'campos'>) {
  const valor = valorEn(atributos, campo.ruta);
  const escribir = (nuevo: unknown) => onChange(conValorEn(atributos, campo.ruta, nuevo));
  const id = campo.ruta.join('.');

  if (campo.tipo === 'objeto') {
    return (
      <fieldset className="grupo">
        <legend>
          {campo.etiqueta}
          {campo.requerido && <span className="requerido"> *</span>}
        </legend>
        {campo.descripcion && <p className="ayuda">{campo.descripcion}</p>}
        {campo.hijos?.map((hijo) => (
          <ControlCampo
            key={hijo.ruta.join('.')}
            campo={hijo}
            atributos={atributos}
            onChange={onChange}
          />
        ))}
      </fieldset>
    );
  }

  if (campo.tipo === 'no-soportado') {
    return (
      <Aviso tono="atencion">
        <strong>{campo.etiqueta}</strong>: este campo del schema no se puede editar desde el panel.
        Se conserva tal cual estaba.
      </Aviso>
    );
  }

  const etiqueta = (
    <>
      {campo.etiqueta}
      {campo.requerido && <span className="requerido"> *</span>}
    </>
  );

  if (campo.tipo === 'multiseleccion') {
    const seleccionados = Array.isArray(valor) ? valor : [];
    return (
      <fieldset className="grupo grupo--opciones">
        <legend>{etiqueta}</legend>
        {campo.descripcion && <p className="ayuda">{campo.descripcion}</p>}
        <div className="opciones">
          {campo.opciones?.map((opcion) => (
            <label key={String(opcion)} className="opcion">
              <input
                type="checkbox"
                checked={seleccionados.includes(opcion)}
                onChange={(evento) => {
                  const restantes = seleccionados.filter((item) => item !== opcion);
                  // Se reconstruye respetando el orden del schema para que dos
                  // ediciones equivalentes produzcan el mismo JSON.
                  escribir(
                    evento.target.checked
                      ? campo.opciones?.filter(
                          (item) => item === opcion || restantes.includes(item),
                        )
                      : restantes,
                  );
                }}
              />
              {String(opcion)}
            </label>
          ))}
        </div>
      </fieldset>
    );
  }

  return (
    <label htmlFor={id}>
      {etiqueta}
      {campo.descripcion && <span className="ayuda">{campo.descripcion}</span>}

      {campo.tipo === 'seleccion' && (
        <select
          id={id}
          value={typeof valor === 'string' || typeof valor === 'number' ? String(valor) : ''}
          onChange={(evento) => escribir(evento.target.value)}
        >
          <option value="">— sin definir —</option>
          {campo.opciones?.map((opcion) => (
            <option key={String(opcion)} value={String(opcion)}>
              {String(opcion)}
            </option>
          ))}
        </select>
      )}

      {campo.tipo === 'texto' && (
        <input
          id={id}
          type="text"
          maxLength={campo.maxLength}
          value={typeof valor === 'string' ? valor : ''}
          onChange={(evento) => escribir(evento.target.value)}
        />
      )}

      {campo.tipo === 'texto-largo' && (
        <textarea
          id={id}
          rows={4}
          maxLength={campo.maxLength}
          value={typeof valor === 'string' ? valor : ''}
          onChange={(evento) => escribir(evento.target.value)}
        />
      )}

      {(campo.tipo === 'numero' || campo.tipo === 'entero') && (
        <input
          id={id}
          type="number"
          step={campo.tipo === 'entero' ? 1 : 'any'}
          min={campo.minimo}
          max={campo.maximo}
          value={typeof valor === 'number' ? valor : ''}
          onChange={(evento) =>
            // Vaciar el campo tiene que borrar la propiedad, no escribir NaN.
            escribir(evento.target.value === '' ? '' : Number(evento.target.value))
          }
        />
      )}
    </label>
  );
}

export function FormularioAtributos({ campos, atributos, onChange }: Props) {
  if (campos.length === 0) {
    return <p className="ayuda">Este reino no define atributos específicos.</p>;
  }

  return (
    <div className="atributos">
      {campos.map((campo) => (
        <ControlCampo
          key={campo.ruta.join('.')}
          campo={campo}
          atributos={atributos}
          onChange={onChange}
        />
      ))}
    </div>
  );
}
