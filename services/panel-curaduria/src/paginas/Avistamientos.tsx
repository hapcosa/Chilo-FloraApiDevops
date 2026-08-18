import { useCallback, useEffect, useState } from "react";
import { useSesion } from "../auth/sesion";
import { Aviso } from "../componentes/Aviso";
import {
  REINOS,
  type Avistamiento,
  type AvistamientoEstado,
  type Reino,
} from "../api/tipos";

const POR_PAGINA = 20;

export function Avistamientos() {
  const { api, usuario } = useSesion();
  const [avistamientos, setAvistamientos] = useState<Avistamiento[]>([]);
  const [total, setTotal] = useState(0);
  const [estado, setEstado] = useState<AvistamientoEstado>("pendiente");
  const [reino, setReino] = useState<Reino | "">("");
  const [offset, setOffset] = useState(0);
  const [error, setError] = useState<string | null>(null);
  const [aviso, setAviso] = useState<string | null>(null);
  const [cargando, setCargando] = useState(true);

  const puedeModerar =
    usuario?.role === "admin" || usuario?.role === "moderator";

  const cargar = useCallback(() => {
    setCargando(true);
    setError(null);
    api
      .avistamientos({ estado, reino, limit: POR_PAGINA, offset })
      .then((respuesta) => {
        setAvistamientos(respuesta.data);
        setTotal(respuesta.pagination?.total ?? respuesta.data.length);
      })
      .catch((fallo) =>
        setError(
          fallo instanceof Error
            ? fallo.message
            : "No se pudieron cargar los avistamientos",
        ),
      )
      .finally(() => setCargando(false));
  }, [api, estado, reino, offset]);

  useEffect(cargar, [cargar]);

  async function moderar(
    avistamiento: Avistamiento,
    decision: "aprobado" | "rechazado",
  ) {
    setError(null);
    setAviso(null);

    let motivo: string | undefined;
    if (decision === "rechazado") {
      const escrito = window.prompt(
        "Motivo del rechazo (lo verá quien reportó):",
      );
      if (escrito === null) return;
      if (escrito.trim() === "") {
        setError("El rechazo necesita un motivo.");
        return;
      }
      motivo = escrito.trim();
    }

    try {
      await api.moderarAvistamiento(avistamiento.id, decision, motivo);
      setAviso(`Avistamiento #${avistamiento.id} ${decision}.`);
      cargar();
    } catch (fallo) {
      setError(fallo instanceof Error ? fallo.message : "No se pudo moderar");
    }
  }

  return (
    <section>
      <header className="cabecera-seccion">
        <h1>Avistamientos</h1>
        <label>
          Estado
          <select
            value={estado}
            onChange={(e) => {
              setEstado(e.target.value as AvistamientoEstado);
              setOffset(0);
            }}
          >
            <option value="pendiente">Pendientes</option>
            <option value="aprobado">Aprobados</option>
            <option value="rechazado">Rechazados</option>
          </select>
        </label>
        <label>
          Reino
          <select
            value={reino}
            onChange={(e) => {
              setReino(e.target.value as Reino | "");
              setOffset(0);
            }}
          >
            <option value="">Todos</option>
            {REINOS.map((valor) => (
              <option key={valor} value={valor}>
                {valor}
              </option>
            ))}
          </select>
        </label>
      </header>

      <p className="ayuda">
        Las fotos de avistamientos viven en un bucket privado y la API solo
        firma subidas, no descargas: aquí se muestra la clave del objeto, no la
        imagen. Para verla hay que abrirla desde la consola del almacenamiento.
      </p>

      {!puedeModerar && (
        <Aviso tono="atencion">
          Moderar avistamientos requiere rol admin o moderator. Puedes
          consultarlos, no resolverlos.
        </Aviso>
      )}
      {error && (
        <Aviso tono="error" desplazar>
          {error}
        </Aviso>
      )}
      {aviso && (
        <Aviso tono="exito" desplazar>
          {aviso}
        </Aviso>
      )}

      {cargando ? (
        <p>Cargando…</p>
      ) : avistamientos.length === 0 ? (
        <p>No hay avistamientos en estado {estado}.</p>
      ) : (
        <table className="tabla">
          <thead>
            <tr>
              <th>#</th>
              <th>Reino</th>
              <th>Sugerido</th>
              <th>Ubicación</th>
              <th>Observado</th>
              <th>Foto (key)</th>
              {estado === "pendiente" && puedeModerar && <th />}
            </tr>
          </thead>
          <tbody>
            {avistamientos.map((avistamiento) => (
              <tr key={avistamiento.id}>
                <td>{avistamiento.id}</td>
                <td>{avistamiento.reino}</td>
                <td className="celda-texto">
                  {avistamiento.nombre_sugerido ??
                    (avistamiento.especie_id
                      ? `especie #${avistamiento.especie_id}`
                      : "—")}
                  {avistamiento.descripcion && (
                    <span className="ayuda">{avistamiento.descripcion}</span>
                  )}
                </td>
                <td>
                  {avistamiento.geo_lat.toFixed(4)},{" "}
                  {avistamiento.geo_lng.toFixed(4)}
                </td>
                <td>{avistamiento.observado_en?.slice(0, 10) ?? "—"}</td>
                <td className="celda-key">{avistamiento.foto_key}</td>
                {estado === "pendiente" && puedeModerar && (
                  <td>
                    <button
                      type="button"
                      onClick={() => moderar(avistamiento, "aprobado")}
                    >
                      Aprobar
                    </button>
                    <button
                      type="button"
                      onClick={() => moderar(avistamiento, "rechazado")}
                    >
                      Rechazar
                    </button>
                  </td>
                )}
              </tr>
            ))}
          </tbody>
        </table>
      )}

      <nav className="paginacion">
        <button
          type="button"
          disabled={offset === 0}
          onClick={() => setOffset(Math.max(0, offset - POR_PAGINA))}
        >
          Anterior
        </button>
        <span>
          {total === 0 ? 0 : offset + 1}–{Math.min(offset + POR_PAGINA, total)}{" "}
          de {total}
        </span>
        <button
          type="button"
          disabled={offset + POR_PAGINA >= total}
          onClick={() => setOffset(offset + POR_PAGINA)}
        >
          Siguiente
        </button>
      </nav>
    </section>
  );
}
