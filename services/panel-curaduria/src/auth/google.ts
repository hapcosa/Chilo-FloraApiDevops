// Google Identity Services, cargado a mano en vez de con una dependencia npm.
//
// Las librerías de React para GIS son envoltorios de veinte líneas sobre un
// script que Google exige cargar desde su CDN de todos modos: no hay nada que
// empaquetar y sí una dependencia más que mantener. El script se inyecta la
// primera vez que hace falta, no en el index.html, para que quien entra con
// correo y clave no pague la petición.

// El mismo ID de cliente *Web* que usa la app móvil: es la audiencia que el
// auth-service valida (`GOOGLE_CLIENT_ID` acepta una lista separada por comas,
// y esta ya está dentro). No es un secreto — viaja en el bundle y en el APK.
const CLIENT_ID_POR_DEFECTO =
  '649341813849-fqtljjtvb4ncfbii7pdt3vbkjd40nsif.apps.googleusercontent.com';

export const googleClientId =
  import.meta.env.VITE_GOOGLE_CLIENT_ID ?? CLIENT_ID_POR_DEFECTO;

const URL_SCRIPT = 'https://accounts.google.com/gsi/client';

interface CredentialResponse {
  credential?: string;
}

interface BotonOpciones {
  theme?: 'outline' | 'filled_blue';
  size?: 'large' | 'medium';
  text?: 'signin_with' | 'continue_with';
  width?: number;
  locale?: string;
}

interface GoogleAccountsId {
  initialize(config: {
    client_id: string;
    callback: (respuesta: CredentialResponse) => void;
    auto_select?: boolean;
    cancel_on_tap_outside?: boolean;
  }): void;
  renderButton(contenedor: HTMLElement, opciones: BotonOpciones): void;
  disableAutoSelect(): void;
}

declare global {
  interface Window {
    google?: { accounts?: { id?: GoogleAccountsId } };
  }
}

let cargando: Promise<GoogleAccountsId> | null = null;

/**
 * Carga el script de GIS una sola vez y resuelve con `google.accounts.id`.
 * Rechaza si el script no carga —red caída, bloqueador— para que el login con
 * correo y clave siga siendo utilizable en vez de quedar la pantalla a medias.
 */
export function cargarGoogleIdentity(): Promise<GoogleAccountsId> {
  const yaCargado = window.google?.accounts?.id;
  if (yaCargado) return Promise.resolve(yaCargado);
  if (cargando) return cargando;

  cargando = new Promise<GoogleAccountsId>((resolver, rechazar) => {
    const script = document.createElement('script');
    script.src = URL_SCRIPT;
    script.async = true;
    script.defer = true;
    script.onload = () => {
      const api = window.google?.accounts?.id;
      if (api) {
        resolver(api);
      } else {
        cargando = null;
        rechazar(new Error('Google Identity Services cargó sin exponer su API'));
      }
    };
    script.onerror = () => {
      // Se limpia la promesa para que un reintento vuelva a inyectar el script
      // en vez de quedarse pegado al primer fallo.
      cargando = null;
      script.remove();
      rechazar(new Error('No se pudo cargar Google. Revisa tu conexión.'));
    };
    document.head.appendChild(script);
  });

  return cargando;
}

/** Borra la preferencia de "entrar siempre con esta cuenta" al cerrar sesión. */
export function olvidarCuentaGoogle(): void {
  window.google?.accounts?.id?.disableAutoSelect();
}
