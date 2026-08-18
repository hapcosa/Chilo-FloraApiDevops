import { useCallback, useEffect, useState } from "react";
import { useSesion } from "../auth/sesion";
import { Aviso } from "../componentes/Aviso";
import type { Categoria, Postulacion, PostulacionEstado } from "../api/tipos";

export function Postulaciones() {
  const { api } = useSesion();
  const [postulaciones, setPostulaciones] = useState<Postulacion[]>([]);
  const [categorias, setCategorias] = useState<Categoria[]>([]);
  const [estado, setEstado] = useState<PostulacionEstado>("pendiente");
  const [error, setError] = useState<string | null>(null);
  const [aviso, setAviso] = useState<string | null>(null);
  const [cargando, setCargando] = useState(true);

  useEffect(() => {
    api
      .categorias()
      .then(setCategorias)
      .catch(() => setCategorias([]));
  }, [api]);

  const cargar = useCallback(() => {
    setCargando(true);
    setError(null);
    api
      .postulaciones(estado)
      .then(setPostulaciones)
      .catch((fallo) =>
        setError(
          fallo instanceof Error
            ? fallo.message
            : "No se pudieron cargar las postulaciones",
        ),
      )
      .finally(() => setCargando(false));
  }, [api, estado]);

  useEffect(cargar, [cargar]);

  const nombreCategoria = (id: number) =>
    categorias.find((categoria) => categoria.id === id)?.nombre ?? `#${id}`;

  async function resolver(
    postulacion: Postulacion,
    decision: "aprobada" | "rechazada",
  ) {
    setError(null);
    setAviso(null);

    let motivo: string | undefined;
    if (decision === "rechazada") {
      // El motivo llega al postulante: rechazar sin explicación es la forma
      // más rápida de perder a alguien que quería ayudar.
      const escrito = window.prompt(
        "Motivo del rechazo (lo verá quien postuló):",
      );
      if (escrito === null) return;
      if (escrito.trim() === "") {
        setError("El rechazo necesita un motivo.");
        return;
      }
      motivo = escrito.trim();
    }

    try {
      await api.resolverPostulacion(postulacion.id, decision, motivo);
      setAviso(
        decision === "aprobada"
          ? `Postulación aprobada: ya cura ${nombreCategoria(postulacion.categoria_id)}.`
          : "Postulación rechazada.",
      );
      cargar();
    } catch (fallo) {
      setError(fallo instanceof Error ? fallo.message : "No se pudo resolver");
    }
  }

  return (
    <section>
      <header className="cabecera-seccion">
        <h1>Postulaciones a curador</h1>
        <label>
          Estado
          <select
            value={estado}
            onChange={(e) => setEstado(e.target.value as PostulacionEstado)}
          >
            <option value="pendiente">Pendientes</option>
            <option value="aprobada">Aprobadas</option>
            <option value="rechazada">Rechazadas</option>
          </select>
        </label>
      </header>

      <p className="ayuda">
        Aprobar añade al usuario como curador de esa categoría. No cambia su rol
        global: sigue siendo una cuenta normal con permiso sobre lo suyo.
      </p>

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
      ) : postulaciones.length === 0 ? (
        <p>
          No hay postulaciones{" "}
          {estado === "pendiente" ? "pendientes" : `en estado ${estado}`}.
        </p>
      ) : (
        <table className="tabla">
          <thead>
            <tr>
              <th>Usuario</th>
              <th>Categoría</th>
              <th>Texto</th>
              <th>Fecha</th>
              {estado === "pendiente" && <th />}
            </tr>
          </thead>
          <tbody>
            {postulaciones.map((postulacion) => (
              <tr key={postulacion.id}>
                <td>#{postulacion.usuario_id}</td>
                <td>{nombreCategoria(postulacion.categoria_id)}</td>
                <td className="celda-texto">{postulacion.texto}</td>
                <td>{postulacion.created_at?.slice(0, 10) ?? "—"}</td>
                {estado === "pendiente" && (
                  <td>
                    <button
                      type="button"
                      onClick={() => resolver(postulacion, "aprobada")}
                    >
                      Aprobar
                    </button>
                    <button
                      type="button"
                      onClick={() => resolver(postulacion, "rechazada")}
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
    </section>
  );
}
