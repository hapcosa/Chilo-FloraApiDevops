import { useEffect, useRef } from "react";

export type TonoAviso = "error" | "exito" | "atencion";

interface AvisoProps {
  tono: TonoAviso;
  // Trae el aviso a la vista si quedó fuera de pantalla. Los formularios y las
  // tablas del panel son largos: al guardar desde el final, el mensaje aparecía
  // arriba de todo y no se veía. No lo activan los avisos permanentes, que
  // moverían la página sola al cargar.
  desplazar?: boolean;
  children: React.ReactNode;
}

export function Aviso({ tono, desplazar = false, children }: AvisoProps) {
  const elemento = useRef<HTMLParagraphElement | null>(null);

  useEffect(() => {
    if (!desplazar) return;
    const nodo = elemento.current;
    if (!nodo) return;

    // Solo se desplaza si el aviso no está ya a la vista: así un re-render con
    // el mismo mensaje no vuelve a mover la página bajo el cursor.
    const caja = nodo.getBoundingClientRect();
    const visible = caja.top >= 0 && caja.bottom <= window.innerHeight;
    if (visible) return;

    const sinAnimacion = window.matchMedia?.(
      "(prefers-reduced-motion: reduce)",
    ).matches;
    nodo.scrollIntoView({
      behavior: sinAnimacion ? "auto" : "smooth",
      block: "center",
    });
  }, [desplazar, children]);

  return (
    <p
      ref={elemento}
      className={`aviso aviso--${tono}`}
      role={tono === "error" ? "alert" : "status"}
      tabIndex={desplazar ? -1 : undefined}
    >
      {children}
    </p>
  );
}
