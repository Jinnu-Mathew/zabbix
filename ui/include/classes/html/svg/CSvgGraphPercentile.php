<?php declare(strict_types = 0);
/*
** Copyright (C) 2001-2024 Zabbix SIA
**
** This program is free software: you can redistribute it and/or modify it under the terms of
** the GNU Affero General Public License as published by the Free Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
** without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License along with this program.
** If not, see <https://www.gnu.org/licenses/>.
**/


class CSvgGraphPercentile extends CSvgGroup {

	private const LABEL_FONT_SIZE = 10;
	private const LABEL_MARGIN = 4;

	private const ZBX_STYLE_CLASS = 'svg-graph-percentile';

	private $color;

	private $side;

	private $label;
	private $value;
	private $min;
	private $max;

	public function __construct($label, $value, $min, $max) {
		parent::__construct();

		$this->label = $label;
		$this->value = $value;
		$this->min = $min;
		$this->max = $max;
	}

	public function setColor(string $color): self {
		$this->color = $color;

		return $this;
	}

	public function setSide(int $side): self {
		$this->side = $side;

		return $this;
	}

	public function makeStyles(): array {
		return [
			'.'.self::ZBX_STYLE_CLASS.'-'.$this->side.' line' => [
				'stroke' => $this->color
			],
			'.'.self::ZBX_STYLE_CLASS.'-'.$this->side.' text' => [
				'font-size' => self::LABEL_FONT_SIZE.'px',
				'fill' => $this->color
			]
		];
	}

	private function draw(): void {
		$total = $this->max - $this->min;

		if ($total == INF) {
			$total = $this->max / 10 - $this->min / 10;
			$fraction = $this->value / 10 - $this->min / 10;
		}
		else {
			$fraction = $this->value - $this->min;
		}

		$y = $this->height + $this->y - CMathHelper::safeMul([
			$this->height, $fraction, 1 / $total
		]);
		$label_x = ($this->side == GRAPH_YAXIS_SIDE_RIGHT)
			? $this->width + $this->x - self::LABEL_MARGIN
			: $this->x + self::LABEL_MARGIN;

		$this->addItem([
			new CSvgLine($this->x, $y, $this->x + $this->width, $y),
			(new CSvgText($this->label, $label_x, $y - self::LABEL_MARGIN / 2))
				->setAttribute('text-anchor', $this->side == GRAPH_YAXIS_SIDE_RIGHT ? 'end' : null)
		]);
	}

	public function toString($destroy = true): string {
		$this
			->addClass(self::ZBX_STYLE_CLASS)
			->addClass(self::ZBX_STYLE_CLASS.'-'.$this->side)
			->draw();

		return parent::toString($destroy);
	}
}
