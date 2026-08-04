import { useCallback, useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import { useSesion } from '../auth/sesion';
import { Aviso } from '../componentes/Aviso';
import type { Especie, EspecieEstado } from '../api/tipos';

const POR_PAGINA = 20;

export function ListadoEspecies() {
  const { api, categoriasCuradas } = useSesion();

  const [especies, setEspecies] = useState<Especie[]>([]);
  const [total, setTotal] = useState(0);
  const [pagina, setPagina] = useState(0);
  const [categoriaId, setCategoriaId] = useState<number | ''>('');
  const [estado, setEstado] = useState<EspecieEstado | ''>('');
  const [busqueda, setBusqueda] = useState('');
  const [error, setError] = useState<string | null>(null);
  const [cargando, setCargando] = useState(true);

  const cargar = useCallback(() => {
    setCargando(true);
    setError(null);
    api
      .especies({
        categoria_id: categoriaId === '' ? undefined : categoriaId,
        estado: estado === '' ? undefined : estado,
        q: busqueda || undefined,
        limit: POR_PAGINA,
        offset: pagina * POR_PAGINA,
      })
      .then((respuesta) => {
        setEspecies(respuesta.data);
        setTotal(respuesta.pagination?.total ?? respuesta.data.length);
      })
      .catch((fallo) => setError(fallo instanceof Error ? fallo.message : 'Error al listar'))
      .finally(() => setCargando(false));
  }, [api, categoriaId, estado, busqueda, pagina]);

  useEffect(cargar, [cargar]);

  async function cambiarEstado(especie: Especie) {
    setError(null);
    try {
      if (especie.estado === 'borrador') await api.publicar(especie.id);
      else await api.despublicar(especie.id);
      cargar();
    } catch (fallo) {
      setError(fallo instanceof Error ? fallo.message : 'No se pudo cambiar el estado');
    }
  }

  const ultimaPagina = Math.max(0, Math.ceil(total / POR_PAGINA) - 1);

  return (
    <section>
      <header className="cabecera-seccion">
        <h1>Especies</h1>
        <Link className="boton" to="/especies/nueva">
          Nueva ficha
        </Link>
      </header>

      <form
        className="filtros"
        onSubmit={(evento) => {
          evento.preventDefault();
          setPagina(0);
          cargar();
        }}
      >
        <label>
          Categoría
          <select
            value={categoriaId}
            onChange={(e) => {
              setCategoriaId(e.target.value === '' ? '' : Number(e.target.value));
              setPagina(0);
            }}
          >
            <option value="">Todas</option>
            {categoriasCuradas.map((categoria) => (
              <option key={categoria.id} value={categoria.id}>
                {categoria.nombre} ({categoria.reino})
              </option>
            ))}
          </select>
        </label>

        <label>
          Estado
          <select
            value={estado}
            onChange={(e) => {
              setEstado(e.target.value as EspecieEstado | '');
              setPagina(0);
            }}
          >
            <option value="">Todos los visibles</option>
            <option value="borrador">Borradores</option>
            <option value="publicada">Publicadas</option>
          </select>
        </label>

        <label>
          Buscar
          <input
            type="search"
            value={busqueda}
            onChange={(e) => setBusqueda(e.target.value)}
            placeholder="nombre científico o común"
          />
        </label>

        <button type="submit">Filtrar</button>
      </form>

      {error && <Aviso tono="error">{error}</Aviso>}

      {cargando ? (
        <p>Cargando…</p>
      ) : especies.length === 0 ? (
        <p>No hay fichas que coincidan con el filtro.</p>
      ) : (
        <table className="tabla">
          <thead>
            <tr>
              <th>Nombre científico</th>
              <th>Común</th>
              <th>Reino</th>
              <th>Estado</th>
              <th />
            </tr>
          </thead>
          <tbody>
            {especies.map((especie) => (
              <tr key={especie.id}>
                <td>
                  <Link to={`/especies/${especie.id}`}>
                    <em>{especie.nombre_cientifico}</em>
                  </Link>
                </td>
                <td>{especie.nombre_comun || '—'}</td>
                <td>{especie.reino}</td>
                <td>
                  <span className={`pastilla pastilla--${especie.estado}`}>{especie.estado}</span>
                </td>
                <td>
                  <button type="button" onClick={() => cambiarEstado(especie)}>
                    {especie.estado === 'borrador' ? 'Publicar' : 'Despublicar'}
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      )}

      <nav className="paginacion">
        <button type="button" disabled={pagina === 0} onClick={() => setPagina(pagina - 1)}>
          Anterior
        </button>
        <span>
          {total} ficha{total === 1 ? '' : 's'} · página {pagina + 1} de {ultimaPagina + 1}
        </span>
        <button
          type="button"
          disabled={pagina >= ultimaPagina}
          onClick={() => setPagina(pagina + 1)}
        >
          Siguiente
        </button>
      </nav>
    </section>
  );
}
