import { createContext, useCallback, useContext, useEffect, useMemo, useState } from 'react';
import type { ReactNode } from 'react';
import {
  crearApi,
  login as loginRequest,
  loginConGoogle as loginGoogleRequest,
  type Api,
} from '../api/api';
import { olvidarCuentaGoogle } from './google';
import type { Categoria, RespuestaLogin, Usuario } from '../api/tipos';

// El token vive en sessionStorage y no en localStorage: sobrevive a un F5 pero
// muere al cerrar la pestaña. Un panel de curaduría abierto en un equipo
// compartido no debería quedar con sesión indefinida.
const CLAVE_TOKEN = 'curaduria.token';
const CLAVE_USUARIO = 'curaduria.usuario';

interface Sesion {
  usuario: Usuario | null;
  token: string | null;
  api: Api;
  // Categorías sobre las que este usuario puede curar. Para admin y moderator
  // son todas: su permiso es global y no pasa por moderador_categorias.
  categoriasCuradas: Categoria[];
  esGlobal: boolean;
  cargandoCategorias: boolean;
  entrar: (email: string, password: string) => Promise<void>;
  // `credential` es el ID token que entrega Google Identity Services.
  entrarConGoogle: (credential: string) => Promise<void>;
  salir: () => void;
}

const SesionContext = createContext<Sesion | null>(null);

function leerUsuarioGuardado(): Usuario | null {
  const crudo = sessionStorage.getItem(CLAVE_USUARIO);
  if (!crudo) return null;
  try {
    return JSON.parse(crudo) as Usuario;
  } catch {
    return null;
  }
}

export function ProveedorSesion({ children }: { children: ReactNode }) {
  const [token, setToken] = useState<string | null>(() => sessionStorage.getItem(CLAVE_TOKEN));
  const [usuario, setUsuario] = useState<Usuario | null>(leerUsuarioGuardado);
  const [categoriasCuradas, setCategoriasCuradas] = useState<Categoria[]>([]);
  const [cargandoCategorias, setCargandoCategorias] = useState(false);

  const salir = useCallback(() => {
    // Sin esto, Google vuelve a entrar solo con la misma cuenta y cerrar sesión
    // no se siente como cerrar sesión.
    olvidarCuentaGoogle();
    sessionStorage.removeItem(CLAVE_TOKEN);
    sessionStorage.removeItem(CLAVE_USUARIO);
    setToken(null);
    setUsuario(null);
    setCategoriasCuradas([]);
  }, []);

  const api = useMemo(() => crearApi(token, salir), [token, salir]);

  const esGlobal = usuario?.role === 'admin' || usuario?.role === 'moderator';

  useEffect(() => {
    if (!usuario || !token) {
      setCategoriasCuradas([]);
      return;
    }
    let vigente = true;
    setCargandoCategorias(true);
    const pedido = esGlobal ? api.categorias() : api.categoriasDe(usuario.id);
    pedido
      .then((categorias) => {
        if (vigente) setCategoriasCuradas(categorias);
      })
      .catch(() => {
        // Un fallo aquí deja el panel sin categorías y por tanto sin permitir
        // editar nada: es el fallo cerrado correcto, no hay que reintentar.
        if (vigente) setCategoriasCuradas([]);
      })
      .finally(() => {
        if (vigente) setCargandoCategorias(false);
      });
    return () => {
      vigente = false;
    };
  }, [api, usuario, token, esGlobal]);

  const guardarSesion = useCallback((respuesta: RespuestaLogin) => {
    sessionStorage.setItem(CLAVE_TOKEN, respuesta.access_token);
    sessionStorage.setItem(CLAVE_USUARIO, JSON.stringify(respuesta.user));
    setToken(respuesta.access_token);
    setUsuario(respuesta.user);
  }, []);

  const entrar = useCallback(
    async (email: string, password: string) => {
      guardarSesion(await loginRequest(email, password));
    },
    [guardarSesion],
  );

  const entrarConGoogle = useCallback(
    async (credential: string) => {
      guardarSesion(await loginGoogleRequest(credential));
    },
    [guardarSesion],
  );

  const valor = useMemo<Sesion>(
    () => ({
      usuario,
      token,
      api,
      categoriasCuradas,
      esGlobal,
      cargandoCategorias,
      entrar,
      entrarConGoogle,
      salir,
    }),
    [
      usuario,
      token,
      api,
      categoriasCuradas,
      esGlobal,
      cargandoCategorias,
      entrar,
      entrarConGoogle,
      salir,
    ],
  );

  return <SesionContext.Provider value={valor}>{children}</SesionContext.Provider>;
}

export function useSesion(): Sesion {
  const sesion = useContext(SesionContext);
  if (!sesion) throw new Error('useSesion fuera de ProveedorSesion');
  return sesion;
}
